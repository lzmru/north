//===--- IR/StmtBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Grammar/Parser.h"
#include "Targets/IRBuilder.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"

#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

namespace north::targets {

using namespace llvm;

llvm::LLVMContext IRBuilder::Context;

Value *IRBuilder::visit(ast::OpenStmt &O) {
  Module->addImport(&O);

  return nullptr;
}

Value *IRBuilder::visit(ast::BlockStmt &Block) {
  type::Scope Scope(CurrentScope, Module);
  CurrentScope = &Scope;

  for (auto Arg : CurrentFn->getArgumentList())
    CurrentScope->addElement(Arg);
  
  Value *Result = nullptr;

  if (auto Body = Block.getBody()) {
    for (auto I = Body->begin(), E = Body->end(); I != E; ++I)
      Result = I->accept(*this);
  }

  if (auto Type = CurrentFn->getTypeIR()) {
    auto InferredType = type::inferFunctionType(*CurrentFn, Module, CurrentScope)->getIR();
    assert(Type);
    
    if (InferredType != Type) {
      auto Pos = CurrentFn->getTypeDecl()->getPosition();

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
          "return value type of `" + CurrentFn->getIdentifier() +  "` does't match the function type", Range);
    }
  } else if (auto TypeDecl = CurrentFn->getTypeDecl()) {
    auto InferredType = type::inferFunctionType(*CurrentFn, Module, CurrentScope)->getIR();
    auto DeclaredType = Module->getType(TypeDecl->getIdentifier())->getIR();
    if (InferredType != DeclaredType) {
      auto Pos = CurrentFn->getTypeDecl()->getPosition();

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
          "return value type of `" + CurrentFn->getIdentifier() +  "` does't match the function type", Range);
    }
  } else {
    if(!isa<ReturnInst>(Result))
      Builder.CreateRetVoid();
  }

  CurrentScope = Scope.getParent();
  return Result;
}

Value *IRBuilder::visit(ast::ReturnStmt &Return) {
  if (auto Expr = Return.getReturnExpr()) {
    GetVal = true;
    return Builder.CreateRet(Expr->accept(*this));
  }
  return Builder.CreateRetVoid();
}

} // namespace north::targets
