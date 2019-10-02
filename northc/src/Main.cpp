//===--- Main.cpp - Compiler driver -----------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Dumper.h"
#include "Grammar/Parser.h"
#include "Targets/IRBuilder.h"

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
#include <Targets/CBuilder.h>

using namespace llvm;
using namespace llvm::sys;

static cl::opt<std::string>
    Path(cl::Positional, cl::desc("<Path to sources>"), cl::Required);

static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Specify output filename"),
                                           cl::value_desc("filename"));

static cl::opt<std::string> OutputTarget("target",
                                         cl::desc("Specify output target"),
                                         cl::value_desc("llvm"), cl::value_desc("c"));

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

  auto MemBuff = llvm::MemoryBuffer::getFile(Path);
  if (auto Error = MemBuff.getError()) {
    llvm::errs() << Path << ": " << Error.message() << '\n';
    return 0;
  }

  if (!MemBuff->get()->getBufferSize())
    return 0;

  llvm::SourceMgr SourceManager;
  SourceManager.AddNewSourceBuffer(std::move(*MemBuff), llvm::SMLoc());

  auto Diagnostic = SourceManager.getDiagHandler();
  SourceManager.setDiagHandler([] (const SMDiagnostic & SMD, void *Context) {
    SMD.print("", errs());
    exit(0);
  });

  north::Lexer Lexer(SourceManager);

  auto Module = new north::type::Module(Path, north::targets::IRBuilder::getContext(), SourceManager);
  north::Parser(Lexer, Module).parse();

  if (EmitAST.getValue()) {
    north::ast::Dumper Dumper;
    applyVisitor(Dumper, Module);
    return 0;
  }

  if (OutputTarget.getValue() == "c") {
    north::targets::CBuilder C(Module, SourceManager);
    applyVisitor(C, Module);
  } else {
    north::targets::IRBuilder IR(Module, SourceManager);
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

    unsigned Opt = 3, Size = 0;
    PMB.OptLevel = Opt;
    PMB.SizeLevel = Size;
    PMB.Inliner = createFunctionInliningPass(Opt, Size, false);
    PMB.DisableUnitAtATime = false;
    PMB.DisableUnrollLoops = false;
    PMB.LoopVectorize = true;
    PMB.SLPVectorize = true;

    /// https://github.com/klee/klee/blob/master/lib/Module/Optimize.cpp
    PM.add(createCFGSimplificationPass());       // Clean up disgusting code
//    PM.add(createPromoteMemoryToRegisterPass()); // Kill useless allocas
    PM.add(createGlobalOptimizerPass());         // Optimize out global vars
    PM.add(createGlobalDCEPass());               // Remove unused fns and globs
    PM.add(createIPConstantPropagationPass());   // IP Constant Propagation
    PM.add(createDeadArgEliminationPass());      // Dead argument elimination
//    PM.add(createInstructionCombiningPass());    // Clean up after IPCP & DAE
    PM.add(createCFGSimplificationPass());       // Clean up after IPCP & DAE

    PM.add(createFunctionInliningPass());  // Inline small functions
    PM.add(createArgumentPromotionPass()); // Scalarize uninlined fn args

//    PM.add(createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(createJumpThreadingPass());        // Thread jumps.
    PM.add(createCFGSimplificationPass());    // Merge & remove BBs
//    PM.add(createInstructionCombiningPass()); // Combine silly seq's

    PM.add(createTailCallEliminationPass()); // Eliminate tail calls
    PM.add(createCFGSimplificationPass());   // Merge & remove BBs
    PM.add(createReassociatePass());         // Reassociate expressions
    PM.add(createLoopRotatePass());
    PM.add(createLICMPass());         // Hoist loop invariants
    PM.add(createLoopUnswitchPass()); // Unswitch loops.

//    PM.add(createInstructionCombiningPass());
    PM.add(createIndVarSimplifyPass());       // Canonicalize indvars
    PM.add(createLoopDeletionPass());         // Delete dead loops
    PM.add(createLoopUnrollPass());           // Unroll small loops
//    PM.add(createInstructionCombiningPass()); // Clean up after the unroller
    PM.add(createGVNPass());                  // Remove redundancies
    PM.add(createMemCpyOptPass());            // Remove memcpy / form memset
    PM.add(createSCCPPass());                 // Constant prop with SCCP

    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
//    PM.add(createInstructionCombiningPass());

    PM.add(createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(createAggressiveDCEPass());        // Delete dead instructions
    PM.add(createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(createStripDeadPrototypesPass());  // Get rid of dead prototypes
    PM.add(createConstantMergePass());        // Merge dup global constants

    TM->adjustPassManager(PMB);

    PMB.populateModulePassManager(PM);

    auto FileType = TargetMachine::CGFT_ObjectFile;

    if (TM->addPassesToEmitFile(PM, dest, nullptr, FileType)) {
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
