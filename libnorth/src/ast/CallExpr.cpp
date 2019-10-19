//===--- AST/CallExpr.cpp ---------------------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/AST.h"
#include "Type/Module.h"

#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/SourceMgr.h>

namespace north::ast {
  
void CallExpr::setCallableFn(FunctionDecl *Fn, type::Module *Module) {
  assert(Fn);
  assert(Module);
  
  if (this->numberOfArgs() != Fn->numberOfArgs() && !Fn->isVarArg()) {
    auto Pos = this->getPosition();

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Module->getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
        llvm::formatv("expected {0} args, not {1}", Fn->numberOfArgs(), this->numberOfArgs()), Range);
  }

  for (size_t I = 0; I < this->numberOfArgs(); ++I) {
    auto CallArg = this->getArg(I);
    
    if (auto FnArg = Fn->getArg(I); FnArg != nullptr && I < Fn->numberOfArgs()) {
      if (FnArg->getNamedArg() != "_") {

        if (CallArg->ArgName == "") {
          auto FnPos = CallArg->Arg->getPosition();

          auto Range = llvm::SMRange(
              llvm::SMLoc::getFromPointer(FnPos.Offset),
              llvm::SMLoc::getFromPointer(FnPos.Offset + FnPos.Length));
          
          Module->getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                     "expected label `" + FnArg->getNamedArg() + "`", Range);
        }

        if (CallArg->ArgName != FnArg->getNamedArg()) {
          auto Arg = CallArg->ArgName;

          auto Range = llvm::SMRange(
              llvm::SMLoc::getFromPointer(Arg.data()),
              llvm::SMLoc::getFromPointer(Arg.data() + Arg.size()));

          Module->getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                     "expected label `" + FnArg->getNamedArg() + "`", Range);
        }
      } else {
        if (CallArg->ArgName != "") {
          auto Arg = CallArg->ArgName;

          auto Range = llvm::SMRange(
              llvm::SMLoc::getFromPointer(Arg.data()),
              llvm::SMLoc::getFromPointer(Arg.data() + Arg.size()));

          Module->getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                     "unexpected label `" + CallArg->ArgName + "`", Range);
        }
      }
    }
  }
  
  this->CallableFn = Fn;
}
  
} // namespace north::ast
