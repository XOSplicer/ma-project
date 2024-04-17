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

// forward decls
run_result_t cells_to_run_result(ide_result_cells_t &result_cells);
run_result_t filter_run_result(run_result_t &run_result);

class RunResult {
  public:
    ide_result_cells_t Ide_result_cells;
    run_result_t Run_result_map;
    run_result_t Run_result_map_filtered;
    RunResult(ide_result_cells_t Ide_result_cells)
      : Ide_result_cells(Ide_result_cells),
        Run_result_map(cells_to_run_result(Ide_result_cells)),
        Run_result_map_filtered(filter_run_result(Run_result_map))
      {};

}; // class RunResult

run_result_t cells_to_run_result(ide_result_cells_t &result_cells)
{
  run_result_t result_map = std::map<const llvm::Value *, std::set<UnsafeDropState>>();
  for (auto cell : result_cells)
  {
    auto llvm_value = cell.getRowKey();
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

void combine_results(RunResult run_1, RunResult run_2)
{
  llvm::outs() << "\n\n###########\n\nCombined run results:\n\n";
  llvm::report_fatal_error("combine_results: unimplemented");
  return;
}

RunResult run_analysis_once(HelperAnalyses &HA, std::vector<std::string> &entrypoints, bool unsafe_construct_as_factory)
{

  llvm::outs() << "Creating problem description and solver\n";
  auto ts_description = UnsafeDropStateDescription(HA, unsafe_construct_as_factory);
  auto ide_ts_problem = createAnalysisProblem<IDETypeStateAnalysis<UnsafeDropStateDescription>>(HA, &ts_description, entrypoints);
  auto ide_solver = IDESolver(ide_ts_problem, &HA.getICFG());
  llvm::outs() << "Solving IDE problem\n";
  auto ide_results = ide_solver.solve();
  llvm::outs() << "IDE results:\n\n";
  ide_results.dumpResults(HA.getICFG());

  llvm::outs() << "Collected results:\n\n";
  ide_result_cells_t result_cells = ide_results.getAllResultEntries();

  auto run_result = RunResult(result_cells);

  for (auto m : run_result.Run_result_map)
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

  Logger::initializeStderrLogger(psr::SeverityLevel::DEBUG);

  if (int err = usage(argc, argv))
  {
    return err;
  }

  std::vector entrypoints = {"main"s};
  HelperAnalyses HA(argv[1], entrypoints);

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
    if (m.second.count(UnsafeDropState::DF_ERROR) || m.second.count(UnsafeDropState::UAF_ERROR)) {
      llvm::outs() << *m.first << " ==> " << m.second << "\n";
    }
  }

  return 0;
}