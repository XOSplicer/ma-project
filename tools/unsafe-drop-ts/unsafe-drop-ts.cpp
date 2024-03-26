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

int usage(int argc, const char **argv) {
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

int main(int argc, const char **argv)
{
  using namespace std::string_literals;

  Logger::initializeStderrLogger(psr::SeverityLevel::DEBUG);

  if (int err = usage(argc, argv)) {
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

  llvm::outs() << "Creating problem description and solver\n";
  auto ts_description = UnsafeDropStateDescription(HA);
  auto ide_ts_problem = createAnalysisProblem<IDETypeStateAnalysis<UnsafeDropStateDescription>>(HA, &ts_description, entrypoints);
  auto ide_solver = IDESolver(ide_ts_problem, &HA.getICFG());
  llvm::outs() << "Solving IDE problem\n";
  auto ide_results = ide_solver.solve();
  llvm::outs() << "IDE results:\n\n";
  ide_results.dumpResults(HA.getICFG());

  return 0;
}