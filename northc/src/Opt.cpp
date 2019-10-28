//===--- Opt.cpp — Configure LLVM optimizations -----------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Opt.h"

#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/CallGraphSCCPass.h>
#include <llvm/Analysis/LoopPass.h>
#include <llvm/Analysis/RegionPass.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils/Mem2Reg.h>

namespace north {

void configureOpimizations(llvm::TargetMachine *TM,
                           north::type::Module *Module,
                           const north::BuildCommand &Command,
                           llvm::raw_fd_ostream &dest) {
  
  llvm::legacy::PassManager PM;
  
  if (Command.Build == north::BuildType::Release) {
    llvm::PassManagerBuilder PMB;
    
    uint8_t Opt = 3;
    uint8_t Size = 0;
    
    PMB.OptLevel = Opt;
    PMB.SizeLevel = Size;
    PMB.Inliner = llvm::createFunctionInliningPass(Opt, Size, false);
    PMB.DisableUnrollLoops = false;
    PMB.LoopVectorize = true;
    PMB.SLPVectorize = true;
    
    /// https://github.com/klee/klee/blob/master/lib/Module/Optimize.cpp
    PM.add(llvm::createCFGSimplificationPass()); // Clean up disgusting code
    //    PM.add(createPromoteMemoryToRegisterPass()); // Kill useless allocas
    PM.add(llvm::createGlobalOptimizerPass()); // Optimize out global vars
    PM.add(llvm::createGlobalDCEPass());       // Remove unused fns and globs
    PM.add(llvm::createIPConstantPropagationPass()); // IP Constant Propagation
    PM.add(llvm::createDeadArgEliminationPass()); // Dead argument elimination
    //    PM.add(createInstructionCombiningPass());    // Clean up after IPCP & DAE
    PM.add(llvm::createCFGSimplificationPass()); // Clean up after IPCP & DAE
    
    PM.add(llvm::createFunctionInliningPass());  // Inline small functions
    PM.add(llvm::createArgumentPromotionPass()); // Scalarize uninlined fn args
    
    //    PM.add(createInstructionCombiningPass()); // Cleanup for scalarrepl.
    PM.add(llvm::createJumpThreadingPass());     // Thread jumps.
    PM.add(llvm::createCFGSimplificationPass()); // Merge & remove BBs
    //    PM.add(createInstructionCombiningPass()); // Combine silly seq's
    
    PM.add(llvm::createTailCallEliminationPass()); // Eliminate tail calls
    PM.add(llvm::createCFGSimplificationPass());   // Merge & remove BBs
    PM.add(llvm::createReassociatePass());         // Reassociate expressions
    PM.add(llvm::createLoopRotatePass());
    PM.add(llvm::createLICMPass());         // Hoist loop invariants
    PM.add(llvm::createLoopUnswitchPass()); // Unswitch loops.
    
    //    PM.add(createInstructionCombiningPass());
    PM.add(llvm::createIndVarSimplifyPass()); // Canonicalize indvars
    PM.add(llvm::createLoopDeletionPass());   // Delete dead loops
    PM.add(llvm::createLoopUnrollPass());     // Unroll small loops
    //    PM.add(createInstructionCombiningPass()); // Clean up after the
    //    unroller
    PM.add(llvm::createGVNPass());       // Remove redundancies
    PM.add(llvm::createMemCpyOptPass()); // Remove memcpy / form memset
    PM.add(llvm::createSCCPPass());      // Constant prop with SCCP
    
    // Run instcombine after redundancy elimination to exploit opportunities
    // opened up by them.
    //    PM.add(createInstructionCombiningPass());
    
    PM.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    PM.add(llvm::createAggressiveDCEPass());        // Delete dead instructions
    PM.add(llvm::createCFGSimplificationPass());    // Merge & remove BBs
    PM.add(llvm::createStripDeadPrototypesPass()); // Get rid of dead prototypes
    PM.add(llvm::createConstantMergePass());       // Merge dup global constants
    
    TM->adjustPassManager(PMB);
    
    PMB.populateModulePassManager(PM);
  }
  
  auto FileType = llvm::TargetMachine::CGFT_ObjectFile;
  
  if (TM->addPassesToEmitFile(PM, dest, nullptr, FileType)) {
    llvm::errs() << "TM can't emit a file of this type";
    return;
  }
  
  PM.run(*Module);
}

} // namespace north
