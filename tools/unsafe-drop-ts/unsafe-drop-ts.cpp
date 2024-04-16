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

using run_result_t = std::map<const llvm::Value *, std::set<UnsafeDropState>>;

void run_analysis_once(HelperAnalyses &HA, std::vector<std::string> &entrypoints, bool unsafe_construct_as_factory)
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
  auto result_cells = ide_results.getAllResultEntries();
  auto result_map = std::map<const llvm::Value *, std::set<UnsafeDropState>>();
  for (auto cell : result_cells)
  {
    auto llvm_value = cell.getRowKey();
    auto state = cell.getValue();
    result_map[llvm_value].emplace(state);
  }
  for (auto m : result_map)
  {
    llvm::outs() << *m.first << " ==> " << m.second << "\n";
  }

  auto result_map_filtered = std::map<const llvm::Value *, std::set<UnsafeDropState>>();
  for (auto m : result_map)
  {
    if (!m.second.count(UnsafeDropState::TS_ERROR) &&
        m.second != std::set({UnsafeDropState::BOT}) &&
        m.second != std::set({UnsafeDropState::UNINIT}) &&
        m.second != std::set({UnsafeDropState::BOT, UnsafeDropState::UNINIT}))
    {
      result_map_filtered.emplace(m.first, m.second);
    }
  }

  llvm::outs() << "\n\n###########\n\nFiltered results:\n\n";
    for (auto m : result_map_filtered)
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
  run_analysis_once(HA, entrypoints, false);

  llvm::outs() << "\n\n###########\n\n Second Run (unsafe_construct_as_factory=true):\n\n";
  run_analysis_once(HA, entrypoints, true);

  return 0;
}