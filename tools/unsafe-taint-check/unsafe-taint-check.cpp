/******************************************************************************
 * Copyright (c) 2017 Philipp Schubert.
 * All rights reserved. This program and the accompanying materials are made
 * available under the terms of LICENSE.txt.
 *
 * Contributors:
 *     Philipp Schubert and others
 *****************************************************************************/


#include "find_unsafe_rs.h"
#include "phasar.h"
#include "llvm/IR/DebugInfo.h"


#include <filesystem>
#include <string>
#include <sstream>

using namespace psr;

int main(int Argc, const char **Argv) {
  using namespace std::string_literals;

  llvm::outs() << "Custom build! Includes find_unsafe_rs lib.\n";

  if (Argc < 2 || !std::filesystem::exists(Argv[1]) ||
      std::filesystem::is_directory(Argv[1])) {
    llvm::errs() << "myphasartool\n"
                    "A small PhASAR-based example program\n\n"
                    "Usage: myphasartool <LLVM IR file>\n";
    return 1;
  }

  std::vector EntryPoints = {"main"s};

  HelperAnalyses HA(Argv[1], EntryPoints);

  if (const auto *F = HA.getProjectIRDB().getFunctionDefinition("main")) {
    // print type hierarchy
    // HA.getTypeHierarchy().print();
    // print points-to information
    // HA.getAliasInfo().print();
    // print inter-procedural control-flow graph
    HA.getICFG().print();

    llvm::outs() << "All functions:\n";
    const llvm::Function *f_llvm_dbg_declare = nullptr;
    for (const llvm::Function *const f : HA.getICFG().getAllFunctions()) {
      llvm::outs() << f->getName() << "\n";
      if (f->getName() == "llvm.dbg.declare") {
        f_llvm_dbg_declare = f;
      }
    }

    auto *find_unsafe_rs = find_unsafe_rs_new();

    llvm::outs() << "Unsafe functions:\n";
    for (const llvm::Function *const f : HA.getICFG().getAllFunctions()) {
      if (f->getName() == "llvm.dbg.declare") {
        continue;
      }
      const llvm::DISubprogram *const sub = f->getSubprogram();
      if (!sub) {
        continue;
      }

      auto path = std::filesystem::path(sub->getDirectory().str()) / sub->getFilename().str();
      int is_unsafe = 0;

      // FIXME: using col=1000 here is very hacky
      int err = find_unsafe_rs_is_any_unsafe_location(find_unsafe_rs, path.c_str(), sub->getLine(), 1000, &is_unsafe);
      if (err) {
          llvm::outs() << "Error: " << err << " with file " << path.string() << "\n";
          continue;
      }
      llvm::outs()
        << f->getName() << ":"
        << path.string() << "::"
        << sub->getLine() << " ["
        << (is_unsafe ? "unsafe" : "safe")
        << "]\n";
    }


    // llvm::outs() << "free(find_unsafe_rs) \n";
    find_unsafe_rs_free(find_unsafe_rs);

    // IFDS template parametrization test
    llvm::outs() << "Testing IFDS:\n";
    auto L = createAnalysisProblem<IFDSSolverTest>(HA, EntryPoints);
    IFDSSolver S(L, &HA.getICFG());
    auto IFDSResults = S.solve();
    // IFDSResults.dumpResults(HA.getICFG());

    // IDE template parametrization test
    llvm::outs() << "Testing IDE:\n";
    auto M = createAnalysisProblem<IDELinearConstantAnalysis>(HA, EntryPoints);
    // Alternative way of solving an IFDS/IDEProblem:
    auto IDEResults = solveIDEProblem(M, HA.getICFG());
    // IDEResults.dumpResults(HA.getICFG());

  } else {
    llvm::errs() << "error: file does not contain a 'main' function!\n";
  }
  return 0;
}