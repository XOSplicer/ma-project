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

#include <filesystem>
#include <string>
#include <sstream>

using namespace psr;

/// @brief Convert IFDS leak map to XTaint leak map for uniform data structure
psr::XTaint::LeakMap_t convert_leaks(std::map<const llvm::Instruction *, std::set<const llvm::Value *>> &leaks)
{
  auto map = psr::XTaint::LeakMap_t();
  for (const auto l : leaks)
  {
    auto set = llvm::SmallSet<const llvm::Value *, 1U>();
    for (const auto v : l.second)
    {
      set.insert(v);
    }
    map.emplace(std::make_pair(l.first, set));
  }
  return map;
}

void print_leaks(psr::XTaint::LeakMap_t &leaks)
{
  for (const auto l : leaks)
  {
    llvm::outs() << "IR: " << *l.first << " -> Leaks values: \n";
    for (const auto v : l.second)
    {
      llvm::outs() << " -> Value: " << *v << " Metadata: " << psr::getMetaDataID(v) << "\n";
    }
  }
  llvm::outs() << "\n";
}

int main(int argc, const char **argv)
{
  using namespace std::string_literals;

  Logger::initializeStderrLogger(psr::SeverityLevel::INFO);

  llvm::outs() << "unsafe-drop-analysis\n\n";

  if (argc < 2 || !std::filesystem::exists(argv[1]) ||
      std::filesystem::is_directory(argv[1]))
  {
    llvm::errs() << "unsafe-drop-analysis \n"
                    "A small PhASAR-based program to check for unsafe drops\n\n"
                    "Usage: unsafe-drop-analysis <LLVM IR file>\n";
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

  PHASAR_LOG_LEVEL(INFO, "Testing IFDS taint analysis with alloc as sources and dealloc as sink");

  TaintConfigData taint_config_data;

  taint_config_data.Functions.push_back(
      FunctionData{
          .Name = "__rust_alloc",
          .ReturnCat = TaintCategory::Source,
      });
  taint_config_data.Functions.push_back(
      FunctionData{
          .Name = "__rust_alloc_zeroed",
          .ReturnCat = TaintCategory::Source,
      });
  taint_config_data.Functions.push_back(
    FunctionData{
      .Name = "__rust_dealloc",
      .HasAllSinkParam = true,
    }
  );

  // set all drop implementations as sinks
  for (const llvm::Function *const f : HA.getICFG().getAllFunctions())
  {
    const llvm::DISubprogram *const sub = f->getSubprogram();
    if (!sub)
    {
      continue;
    }
    auto name = sub->getName().split("<").first;
    if (name.contains("drop"))
    {
      PHASAR_LOG_LEVEL(INFO, "drop implementation found (w/o <...>): " << name);
      taint_config_data.Functions.push_back(
          FunctionData{
              .Name = sub->getLinkageName().str(),
              .HasAllSinkParam = true,
          });
    }
  }

  LLVMTaintConfig taint_config(HA.getProjectIRDB(), taint_config_data);
  llvm::outs() << "taint config:\n"
               << taint_config << "\n";

  IFDSTaintAnalysis ifds_taint_problem(&HA.getProjectIRDB(), &HA.getAliasInfo(), &taint_config);

  PHASAR_LOG_LEVEL(INFO, "Solving IFDSTaintAnalysis taint problem");
  IFDSSolver S(ifds_taint_problem, &HA.getICFG());
  auto IFDSResults = S.solve();
  IFDSResults.dumpResults(HA.getICFG());

  auto ifds_taint_leaks = convert_leaks(ifds_taint_problem.Leaks);
  llvm::outs() << "\n"
               << ifds_taint_leaks.size()
               << " leaks found using IFDS Taint:\n";
  print_leaks(ifds_taint_leaks);

  PHASAR_LOG_LEVEL(INFO, "Testing IDE extended taint analysis with unsafe functions as source:");

  const std::vector<std::string> entry_points = {"main"};
  auto ide_xtaint_problem =
      createAnalysisProblem<IDEExtendedTaintAnalysis<>>(HA, taint_config, entry_points);

  PHASAR_LOG_LEVEL(INFO, "Solving IDEXTaintAnalysis taint problem");
  IDESolver IDE_S(ide_xtaint_problem, &HA.getICFG());
  auto IDEResults = IDE_S.solve();

  auto ide_xtaint_leaks = ide_xtaint_problem.getAllLeaks(IDEResults);
  llvm::outs() << "\n"
               << ide_xtaint_leaks.size()
               << " leaks found using IDE XTaint:\n";
  print_leaks(ide_xtaint_leaks);

  // check for a value that is leaked twice
  PHASAR_LOG_LEVEL(INFO, "Checking for double leak of values:\n");

  auto leaked_values = llvm::DenseSet<const llvm::Value *>();
  for (const auto leak : ide_xtaint_leaks)
  {
    for (const auto value : leak.second)
    {
      leaked_values.insert(value);
    }
  }

  auto leaks_per_value = llvm::DenseMap<const llvm::Value *, llvm::SmallSet<const llvm::Instruction *, 2U>>();
  for (const auto value : leaked_values)
  {
    auto instructions = llvm::SmallSet<const llvm::Instruction *, 2U>();
    for (auto const leak : ide_xtaint_leaks)
    {
      if (leak.second.contains(value))
      {
        instructions.insert(leak.first);
      }
    }
    leaks_per_value.insert(std::make_pair(value, instructions));
  }

  for (const auto value_instrs : leaks_per_value)
  {
    if (value_instrs.second.size() >= 2)
    {
      llvm::outs() << "Value: " << *value_instrs.first << "\n"
                   << "is leaked multiple times by instructions: \n";
      for (auto const instr : value_instrs.second)
      {
        llvm::outs() << " -> " << *instr << " ( Opcode = " << instr->getOpcodeName() << " ) \n";
        if (auto const *cb = llvm::dyn_cast<llvm::CallBase>(instr))
          llvm::outs() << *cb << " => called function: " << cb->getCalledFunction()->getName() << "\n";
      }
      llvm::outs() << "\n";
    }
  }

  return 0;
}