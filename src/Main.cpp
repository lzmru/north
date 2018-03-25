//===--- Main.cpp - Compiler driver -----------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/Dumper.h"
#include "Grammar/Parser.h"
#include "IR/IRBuilder.h"

#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/CallGraphSCCPass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Analysis/RegionPass.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

#include <clang/Basic/FileManager.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/Support/CommandLine.h>

#include <llvm/Support/Casting.h>

using namespace llvm;
using namespace llvm::sys;

static cl::opt<std::string>
    FileName(cl::Positional, cl::desc("<Path to sources>"), cl::Required);
static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Specify output filename"),
                                           cl::value_desc("filename"));

static cl::opt<bool> EmitAST("emit-ast",
                             cl::desc("Emit AST for input source file"));
static cl::opt<bool> EmitLLVM("emit-llvm",
                              cl::desc("Emit LLVM IR for input source file"));

void applyVisitor(north::ast::Visitor &V, north::type::Module *M) {
  for (auto I = M->getAST()->begin(), E = M->getAST()->end(); I != E; ++I)
    I->accept(V);
}

int main(int argc, const char *argv[]) {

  cl::ParseCommandLineOptions(argc, argv, "The North language compiler\n");

  StringMap<Module *> ModuleMap;
  north::Parser parser(FileName.c_str());
  auto Module = parser.parse();

  if (EmitAST.getValue()) {
    north::ast::Dumper Dumper;
    applyVisitor(Dumper, Module);
  } else {
    north::ir::IRBuilder IR(Module);
    applyVisitor(IR, Module);

    verifyModule(*Module, &outs());

    if (EmitLLVM.getValue()) {
      llvm::outs() << *Module;
      return 0;
    }

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getDefaultTargetTriple();
    Module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
      errs() << Error;
      return 1;
    }

    auto CPU = "generic", Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TM = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    Module->setDataLayout(TM->createDataLayout());

    std::string Filename = Module->getModuleIdentifier() + ".o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

    if (EC) {
      errs() << "Couldn't open file: " << EC.message();
      return 1;
    }

    legacy::PassManager PM;
    PassManagerBuilder PMB;
    legacy::FunctionPassManager FPM(Module);

    unsigned Opt = 3, Size = 0;
    PMB.OptLevel = Opt;
    PMB.SizeLevel = Size;
    PMB.Inliner = createFunctionInliningPass(Opt, Size, false);
    PMB.DisableUnitAtATime = false;
    PMB.DisableUnrollLoops = false;
    PMB.LoopVectorize = true;
    PMB.SLPVectorize = true;

    FPM.add(createPromoteMemoryToRegisterPass());
    FPM.add(createInstructionCombiningPass());
    FPM.add(createReassociatePass());
    FPM.add(createGVNPass());
    FPM.add(createCFGSimplificationPass());

    FPM.doInitialization();

    for (auto &F : Module->getFunctionList())
      FPM.run(F);

    TM->adjustPassManager(PMB);

    PMB.populateModulePassManager(PM);
    PMB.populateFunctionPassManager(FPM);

    auto FileType = TargetMachine::CGFT_ObjectFile;

    if (TM->addPassesToEmitFile(PM, dest, FileType)) {
      errs() << "TM can't emit a file of this type";
      return 1;
    }

    PM.run(*Module);
    dest.flush();

    system(("gcc " + Filename + " -o " +
            ((OutputFilename.getValue() == "") ? Module->getModuleIdentifier()
                                               : OutputFilename.getValue()))
               .c_str());
  }

  return 0;
}
