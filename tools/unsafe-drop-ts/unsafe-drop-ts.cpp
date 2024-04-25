/******************************************************************************
 * Copyright (c) 2023 Felix Stegmaier.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *  Template of `myphasartool` by Philipp Schubert and others under MIT License
 *****************************************************************************/

#include "phasar.h"
#include "llvm/IR/DebugInfo.h"
#include "UnsafeDropStateDescription.h"

#include <filesystem>
#include <string>
#include <sstream>

#define DEBUG_IR_FILE "/workspaces/ma-project/build/analysis-targets/unsafe_tests_uaf_slice_01-0083f63475ad221d.ll"
// #define DEBUG_MODE
#undef DEBUG_MODE

using namespace psr;

int usage(int argc, const char **argv)
{
  llvm::outs() << "unsafe-drop-ts\n\n";
  if (argc < 2 || !std::filesystem::exists(argv[1]) ||
      std::filesystem::is_directory(argv[1]))
  {
    llvm::errs() << "unsafe-drop-ts \n"
                    "A small PhASAR-based program to check for unsafe drops using typestate analysis\n\n"
                    "Usage: unsafe-drop-ts <LLVM IR file>\n";
    return 1;
  }
  return 0;
}

llvm::raw_ostream &operator<<(llvm::raw_ostream &OS, const std::set<UnsafeDropState> &S)
{
  OS << "[ ";
  for (auto s : S)
  {
    OS << to_string(s) << ", ";
  }
  OS << "]";
  return OS;
}

using ide_result_cells_t = std::vector<psr::Table<const llvm::Instruction *, const llvm::Value *, psr::UnsafeDropState>::Cell>;
using run_result_t = std::map<const llvm::Value *, std::set<UnsafeDropState>>;
using ide_results_t = psr::SolverResults<const llvm::Instruction *, const llvm::Value *, psr::UnsafeDropState>;

// forward decls
run_result_t cells_to_run_result(ide_result_cells_t &result_cells);
run_result_t filter_run_result(run_result_t &run_result);

class RunResult
{
public:
  ide_results_t Ide_results;
  ide_result_cells_t Ide_result_cells;
  run_result_t Run_result_map;
  run_result_t Run_result_map_filtered;
  RunResult(ide_results_t Ide_results)
      : Ide_results(Ide_results),
        Ide_result_cells(Ide_results.getAllResultEntries()),
        Run_result_map(cells_to_run_result(Ide_result_cells)),
        Run_result_map_filtered(filter_run_result(Run_result_map)){};
}; // class RunResult

run_result_t cells_to_run_result(ide_result_cells_t &result_cells)
{
  run_result_t result_map = std::map<const llvm::Value *, std::set<UnsafeDropState>>();
  for (auto cell : result_cells)
  {
    // auto llvm_value = cell.getRowKey();
    auto llvm_value = cell.getColumnKey();
    auto state = cell.getValue();
    result_map[llvm_value].emplace(state);
  }
  return result_map;
}

run_result_t filter_run_result(run_result_t &run_result)
{
  auto result_map_filtered = std::map<const llvm::Value *, std::set<UnsafeDropState>>();
  for (auto m : run_result)
  {
    if (!m.second.count(UnsafeDropState::TS_ERROR) &&
        m.second != std::set({UnsafeDropState::BOT}) &&
        m.second != std::set({UnsafeDropState::UNINIT}) &&
        m.second != std::set({UnsafeDropState::BOT, UnsafeDropState::UNINIT}))
    {
      result_map_filtered.emplace(m.first, m.second);
    }
  }
  return result_map_filtered;
}

void combine_results(HelperAnalyses &HA, const RunResult &run_1, const RunResult &run_2)
{
  std::unordered_set<const llvm::Value *> run_2_error_values;
  for (const auto m : run_2.Run_result_map_filtered)
  {
    if (m.second.count(UnsafeDropState::DF_ERROR) || m.second.count(UnsafeDropState::UAF_ERROR))
    {
      run_2_error_values.insert(m.first);
    }
  }

  std::unordered_set<const llvm::Value *> run_1_wrapped_values;
  for (const auto m : run_1.Run_result_map_filtered)
  {
    if (m.second.count(UnsafeDropState::RAW_WRAPPED))
    {
      run_1_wrapped_values.insert(m.first);
    }
  }

  auto description = UnsafeDropStateDescription(HA);

  // TODO: implement
  // this should for each value that ends in a DF/UAF error state
  // find all values that are passed into the unsafeConstruct call
  // where the value has been tagged as coming from getPtr and therefore is also in state RAW_WRAPPED

  // FIXME: is this actually the correct logic????? dont we under approximate the problem with the general approach of 2 analysis

  for (const auto instr : HA.getProjectIRDB().getAllInstructions())
  {

    // get run_2 value / state pairs for each instruction, filter by DF/UAF ERROR state

    // SEGFAULT happens in this method call
    // auto res_2 = run_2.Ide_results.resultsAtInLLVMSSA(instr, true, true);
    // avoid calling resultsAtInLLVMSSA(instr), as it segfaults
    // maybe instead call resultAtInLLVMSSA(instr) with each of the run_2_error_values
    // avoid calling resultAtInLLVMSSA, as it also segfaults

    // NOTE: the association of facts to the instruction mithgt be of by one,
    // as the fact only holds after the instructions that introduced it.

    std::unordered_map<const llvm::Value *, UnsafeDropState> res_2_filtered;
    for (const auto cell : run_2.Ide_result_cells)
    {
      if (cell.getRowKey() != instr) { continue; }
      auto llvm_value = cell.getColumnKey();
      auto state = cell.getValue();
      if (run_2_error_values.count(llvm_value))
      {
        res_2_filtered.emplace(llvm_value, state);
      }
    }

    // get run_1 value / state pairs for each instruction, filter by RAW_WRAPPED state
    // this might segfault
    // auto res_1 = run_1.Ide_results.resultsAtInLLVMSSA(instr, true, true);

    std::unordered_map<const llvm::Value *, UnsafeDropState> res_1_filtered;
    for (const auto cell : run_1.Ide_result_cells)
    {
      if (cell.getRowKey() != instr) { continue; }
      auto llvm_value = cell.getColumnKey();
      auto state = cell.getValue();
      if (run_1_wrapped_values.count(llvm_value))
      {
        res_1_filtered.emplace(llvm_value, state);
      }
    }


    // merge the two sets on the value
    std::unordered_map<const llvm::Value *, llvm::SmallSet<UnsafeDropState, 2>> joined;
    for (const auto m_1 : res_1_filtered)
    {
      // operator[] will default construct an empty set value if not present
      joined[m_1.first].insert(m_1.second);
    }
    for (const auto m_2 : res_2_filtered)
    {
      // operator[] will default construct an empty set value if not present
      joined[m_2.first].insert(m_2.second);
    }

    std::set<UnsafeDropState> all_states;
    for (const auto m : joined)
    {
      for (const auto s : m.second)
      {
        all_states.insert(s);
      }
    }

    // llvm::outs() << "States at instruction: " << *instr << " ==\n==> " << all_states << "\n";


    // TODO: check if RAW_WRAPPED and error states exists at the same instruction of kind unsafeConstruct
    // and RAW_WRAPPED is used as a arg
    // FIXME: should check for arg

    // TODO: do only compute when unsafeConstruct invoke or all instr
    /*
    if (description.funcNameToToken(llvm::demangle(???)) != UnsafeDropToken::UNSAFE_CONSTRUCT)
    {
      continue;
    }
    */

    if ((all_states.count(UnsafeDropState::DF_ERROR) || all_states.count(UnsafeDropState::UAF_ERROR)) && all_states.count(UnsafeDropState::RAW_WRAPPED))
    {
      llvm::outs() << "\n\nPotential error detected at instruction:\n    "
                   << *instr
                   << "\n Joined states: \n    ";
      for (const auto m : joined)
      {
        llvm::outs() << *m.first << " ==> " << m.second << "\n";
      }
    }
  }

  return;
}

RunResult run_analysis_once(HelperAnalyses &HA, const std::vector<std::string> &entrypoints, const bool unsafe_construct_as_factory)
{

  llvm::outs() << "Creating problem description and solver\n";
  const auto ts_description = UnsafeDropStateDescription(HA, unsafe_construct_as_factory);
  auto ide_ts_problem = createAnalysisProblem<IDETypeStateAnalysis<UnsafeDropStateDescription>>(HA, &ts_description, entrypoints);
  auto ide_solver = IDESolver(ide_ts_problem, &HA.getICFG());
  llvm::outs() << "Solving IDE problem\n";
  auto ide_results = ide_solver.solve();
  llvm::outs() << "IDE results:\n\n";
  ide_results.dumpResults(HA.getICFG());

  llvm::outs() << "Collected results:\n\n";

  const auto run_result = RunResult(ide_results);

  for (const auto m : run_result.Run_result_map)
  {
    llvm::outs() << *m.first << " ==> " << m.second << "\n";
  }

  return run_result;
}

void print_run_result(run_result_t &run_result)
{
  llvm::outs() << "\n\n###########\n\nFiltered run results:\n\n";
  for (auto m : run_result)
  {
    llvm::outs() << *m.first << " ==> " << m.second << "\n";
  }
}

int main(int argc, const char **argv)
{
  using namespace std::string_literals;

  // Logger::initializeStderrLogger(psr::SeverityLevel::DEBUG);
  Logger::initializeStderrLogger(psr::SeverityLevel::INFO);

  if (const int err = usage(argc, argv))
  {
    // FIXME: change back after debugging
    PHASAR_LOG_LEVEL(CRITICAL, "error: incorrect usage");
#ifdef DEBUG_MODE
    PHASAR_LOG_LEVEL(WARNING, "will continue for debug mode");
#else
    return err;
#endif
  }

#ifdef DEBUG_MODE
  std::string IRFile = DEBUG_IR_FILE;
#else
  std::string IRFile = argv[1];
#endif

  const std::vector entrypoints = {"main"s};
  HelperAnalyses HA(IRFile, entrypoints);

  const auto *F = HA.getProjectIRDB().getFunctionDefinition("main");
  if (!F)
  {
    PHASAR_LOG_LEVEL(CRITICAL, "error: file does not contain a 'main' function!");
    return 1;
  }

  llvm::outs() << "\n\n###########\n\n First Run (unsafe_construct_as_factory=false):\n\n";
  auto run_1 = run_analysis_once(HA, entrypoints, false);
  print_run_result(run_1.Run_result_map_filtered);

  llvm::outs() << "\n\n###########\n\n Second Run (unsafe_construct_as_factory=true):\n\n";
  auto run_2 = run_analysis_once(HA, entrypoints, true);
  print_run_result(run_2.Run_result_map_filtered);

  llvm::outs() << "\n\n###########\n\nResults with DF/UAF Errors:\n\n";
  for (auto m : run_2.Run_result_map_filtered)
  {
    if (m.second.count(UnsafeDropState::DF_ERROR) || m.second.count(UnsafeDropState::UAF_ERROR))
    {
      llvm::outs() << *m.first << " ==> " << m.second << "\n";
    }
  }

  llvm::outs() << "\n\n###########\n\nResults with DF/UAF Errors that have also been RAW_WRAPPED or RAW_REFERENCED:\n\n";
  for (auto m : run_2.Run_result_map_filtered)
  {
    if (
      (m.second.count(UnsafeDropState::DF_ERROR) || m.second.count(UnsafeDropState::UAF_ERROR))
      && (m.second.count(UnsafeDropState::RAW_REFERENCED) || m.second.count(UnsafeDropState::RAW_WRAPPED))
    )
    {
      llvm::outs() << *m.first << " ==> " << m.second << "\n";
    }
  }

  // check with values from first run aswell by joining the two runs on the value

  run_result_t merged_result;
  for (const auto m_1 : run_1.Run_result_map_filtered)
  {
    for (const auto s : m_1.second)
    {
      // operator[] will default construct an empty set value if not present
      merged_result[m_1.first].insert(s);
    }
  }
  for (const auto m_2 : run_2.Run_result_map_filtered)
  {
    for (const auto s : m_2.second)
    {
      // operator[] will default construct an empty set value if not present
      merged_result[m_2.first].insert(s);
    }
  }

    llvm::outs() << "\n\n###########\n\n===Merged=== Results with DF/UAF Errors that have also been RAW_WRAPPED or RAW_REFERENCED:\n\n";
  for (auto m : run_2.Run_result_map_filtered)
  {
    if (
      (m.second.count(UnsafeDropState::DF_ERROR) || m.second.count(UnsafeDropState::UAF_ERROR))
      && (m.second.count(UnsafeDropState::RAW_REFERENCED) || m.second.count(UnsafeDropState::RAW_WRAPPED))
    )
    {
      llvm::outs() << *m.first << " ==> " << m.second << "\n";
    }
  }


  llvm::outs() << "\n\n###########\n\nCombined results:\n\n";
  llvm::outs() << "(skipped)\n";
  // combine_results(HA, run_1, run_2);

  llvm::outs() << "Done.\n\n";
  return 0;
}