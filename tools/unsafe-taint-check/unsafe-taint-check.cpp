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

std::vector<llvm::Function const *> get_unsafe_functions(HelperAnalyses &HA)
{
  auto *find_unsafe_rs = find_unsafe_rs_new();
  std::vector<llvm::Function const *> unsafe_functions;
  llvm::outs() << "\nDeclared functions [safety]:";
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
      PHASAR_LOG_LEVEL(WARNING, "Error in file " << path.string() << " (error " << err << " ): " << strerror(err));
      continue;
    }
    llvm::outs()
        << f->getName()
        << " ('" << sub->getName() << "')"
        << " : " << path.string() << ":"
        << sub->getLine() << " ["
        << (is_unsafe ? "unsafe" : "safe")
        << "]\n";
    if (is_unsafe)
    {
      unsafe_functions.push_back(f);
    }
  }
  find_unsafe_rs_free(find_unsafe_rs);
  return unsafe_functions;
}

int main(int argc, const char **argv)
{
  using namespace std::string_literals;

  Logger::initializeStderrLogger(psr::SeverityLevel::INFO);

  llvm::outs() << "unsafe-taint-check with find_unsafe_rs lib\n\n";

  if (argc < 2 || !std::filesystem::exists(argv[1]) ||
      std::filesystem::is_directory(argv[1]))
  {
    llvm::errs() << "unsafe-taint-check \n"
                    "A small PhASAR-based program to check the unsafe taint for rust\n\n"
                    "Usage: unsafe-taint-check <LLVM IR file>\n";
    return 1;
  }

  std::vector entrypoints = {"main"s};

  HelperAnalyses HA(argv[1], entrypoints);

  const auto *F = HA.getProjectIRDB().getFunctionDefinition("main");
  if (!F)
  {
    PHASAR_LOG_LEVEL(CRITICAL, "error: file does not contain a 'main' function!");
    return 1;
  }

  // HA.getICFG().print();
  auto unsafe_functions = get_unsafe_functions(HA);
  llvm::outs() << "\nUnsafe functions:\n";
  for (auto f : unsafe_functions)
  {
    llvm::outs() << f->getName() << "\n";
  }

  PHASAR_LOG_LEVEL(INFO, "Testing IFDS taint analysis with unsafe functions as source:");

  TaintConfigData taint_config_data;
  for (auto f : unsafe_functions)
  {
    // all sret(%Type) marked arguments are considered return values and therefore tainted
    // fill vec<int> with int if f->args()[i].getParamStructRetType() != NULL
    std::vector<unsigned int> sretArgs;
    unsigned int arg_num = 0;
    for (auto &arg : f->args())
    {
      if (arg.hasStructRetAttr())
      {
        sretArgs.push_back(arg_num);
      }
      arg_num++;
    }

    taint_config_data.Functions.push_back(
        FunctionData{
            .Name = f->getName().str(),
            .ReturnCat = TaintCategory::Source,
            .SourceValues = sretArgs,
        });
  }
  taint_config_data.Functions.push_back(
      FunctionData{
          .Name = "sink",
          .HasAllSinkParam = true,
      });
  LLVMTaintConfig taint_config(HA.getProjectIRDB(), taint_config_data);
  llvm::outs() << "taint config:\n"
               << taint_config << "\n";

  IFDSTaintAnalysis ifds_taint_problem(&HA.getProjectIRDB(), &HA.getAliasInfo(), &taint_config);

  PHASAR_LOG_LEVEL(INFO, "Solving IFDSTaintAnalysis taint problem");
  IFDSSolver S(ifds_taint_problem, &HA.getICFG());
  auto IFDSResults = S.solve();
  // IFDSResults.dumpResults(HA.getICFG());

  llvm::outs() << "\n"
               << ifds_taint_problem.Leaks.size() << " leaks found using IFDS Taint:\n";
  for (const auto l : ifds_taint_problem.Leaks)
  {
    llvm::outs() << "IR: " << *l.first << " -> Leaks values: \n";
    for (const auto v : l.second)
    {
      llvm::outs() << " -> Value: " << *v << "\n";
    }
  }
  llvm::outs() << "\n";

  PHASAR_LOG_LEVEL(INFO, "Testing IDE extended taint analysis with unsafe functions as source:");

  const std::vector<std::string> entry_points = {"main"};
  auto ide_xtaint_problem =
      createAnalysisProblem<IDEExtendedTaintAnalysis<>>(HA, taint_config, entry_points);

  PHASAR_LOG_LEVEL(INFO, "Solving IDEXTaintAnalysis taint problem");
  IDESolver IDE_S(ide_xtaint_problem, &HA.getICFG());
  auto IDEResults = IDE_S.solve();

  llvm::outs() << "\n"
               << ide_xtaint_problem.getAllLeaks(IDEResults).size() << " leaks found using IDE XTaint:\n";
  for (const auto l : ide_xtaint_problem.getAllLeaks(IDEResults))
  {
    llvm::outs() << "IR: " << *l.first << " -> Leaks values: \n";
    for (const auto v : l.second)
    {
      llvm::outs() << " -> Value: " << *v << "\n";
    }
  }
  llvm::outs() << "\n";

  return 0;
}