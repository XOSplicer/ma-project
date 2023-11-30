/******************************************************************************
 * Copyright (c) 2023 Felix Stegmaier.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *  Template of `myphasartool` by Philipp Schubert and others under MIT License
 *****************************************************************************/

#include "find_unsafe_rs.h"
#include "phasar.h"
#include "llvm/IR/DebugInfo.h"

#include <filesystem>
#include <string>
#include <sstream>

using namespace psr;

int main(int Argc, const char **Argv)
{
  using namespace std::string_literals;

  llvm::outs() << "unsafe-taint-check with find_unsafe_rs lib\n\n";

  if (Argc < 2 || !std::filesystem::exists(Argv[1]) ||
      std::filesystem::is_directory(Argv[1]))
  {
    llvm::errs() << "unsafe-taint-check \n"
                    "A small PhASAR-based program to check the unsafe taint for rust\n\n"
                    "Usage: unsafe-taint-check <LLVM IR file>\n";
    return 1;
  }

  std::vector EntryPoints = {"main"s};

  HelperAnalyses HA(Argv[1], EntryPoints);

  const auto *F = HA.getProjectIRDB().getFunctionDefinition("main");
  if (!F)
  {
    llvm::errs() << "error: file does not contain a 'main' function!\n";
    return 1;
  }

  HA.getICFG().print();

  auto *find_unsafe_rs = find_unsafe_rs_new();

  std::vector<llvm::Function const *> unsafe_functions;
  llvm::outs() << "\n\nDeclared functions [safety]:\n";
  for (const llvm::Function *const f : HA.getICFG().getAllFunctions())
  {
    if (f->getName() == "llvm.dbg.declare")
    {
      continue;
    }
    const llvm::DISubprogram *const sub = f->getSubprogram();
    if (!sub)
    {
      continue;
    }

    auto path = std::filesystem::path(sub->getDirectory().str()) / sub->getFilename().str();
    int is_unsafe = 0;
    // FIXME: using col=1000 here is very hacky
    int err = find_unsafe_rs_is_any_unsafe_location(
        find_unsafe_rs, path.c_str(), sub->getLine(), 1000, &is_unsafe);
    if (err)
    {
      llvm::outs() << "Error in file " << path.string()
                   << " (error " << err << " ): " << strerror(err) << "\n";
      continue;
    }
    llvm::outs()
        << f->getName()
        << " ('" << sub->getName() << "')"
        << " : " << path.string() << "::"
        << sub->getLine() << " ["
        << (is_unsafe ? "unsafe" : "safe")
        << "]\n";
    if (is_unsafe)
    {
      unsafe_functions.push_back(f);
    }
  }

  llvm::outs() << "\n\nUnsafe functions:\n";
  for (auto f: unsafe_functions) {
    llvm::outs() << f->getName() << "\n";
  }

  // llvm::outs() << "free(find_unsafe_rs) \n";
  find_unsafe_rs_free(find_unsafe_rs);

  // IFDS template parametrization test
  llvm::outs() << "\n\nTesting IFDS:\n";
  auto L = createAnalysisProblem<IFDSSolverTest>(HA, EntryPoints);
  IFDSSolver S(L, &HA.getICFG());
  auto IFDSResults = S.solve();
  // IFDSResults.dumpResults(HA.getICFG());

  // IDE template parametrization test
  llvm::outs() << "\n\nTesting IDE:\n";
  auto M = createAnalysisProblem<IDELinearConstantAnalysis>(HA, EntryPoints);
  // Alternative way of solving an IFDS/IDEProblem:
  auto IDEResults = solveIDEProblem(M, HA.getICFG());
  // IDEResults.dumpResults(HA.getICFG());

  return 0;
}