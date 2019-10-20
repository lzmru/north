//===--- IR/DeclBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Targets/IRBuilder.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>

#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/SourceMgr.h>

namespace north::targets {

using namespace llvm;

Value *IRBuilder::visit(ast::FunctionDecl &Fn) {
  if (!Fn.getBlockStmt())
    return nullptr;
  
  auto BB = BasicBlock::Create(Context, "entry", Fn.getIR());
  Builder.SetInsertPoint(BB);
  CurrentFn = &Fn;

  return Fn.getBlockStmt()->accept(*this);
}
  
llvm::Value *IRBuilder::visit(ast::GenericFunctionDecl &GenericFn) {
  for (auto Callee : GenericFn.getCalls()) {
    auto Fn = GenericFn.instantiate(Callee, Module);
    if (!Fn->maybeGetIR()) {
      Fn->createIR(Module);
      Callee->setCallableFn(Fn, Module);
      Fn->accept(*this);
    } else {
      Callee->setCallableFn(Fn, Module);
    }
  }
  return nullptr;
}

Value *IRBuilder::visit(ast::InterfaceDecl &) { return nullptr; }

Value *IRBuilder::visit(ast::VarDecl &Var) {
  llvm::Type *Type = nullptr;

  if (auto TypeDecl = Var.getType()) {
    Type = Module->getType(Var.getType()->getIdentifier())->getIR();
    if (TypeDecl->isPtr())
      Type = Type->getPointerTo(0);

    auto InferredType = inferVarType(Var, Module, CurrentScope)->getIR();
    if (InferredType != Type) {
      auto Pos = Var.getPosition();

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
          "type of value `" + Var.getIdentifier() +  "` type does't match the variable type", Range);
    }
  } else {
    Type = inferVarType(Var, Module, CurrentScope)->getIR();
    Var.setIRType(Type);
  }

  auto IR = Builder.CreateAlloca(Type, nullptr, Var.getIdentifier());
  Var.setIRValue(IR);
  CurrentScope->addElement(&Var);

  if (auto Val = Var.getValue())
    Builder.CreateStore(Val->accept(*this), IR);

  return IR;
}

Value *IRBuilder::visit(ast::AliasDecl &Alias) { return nullptr; }

Value *IRBuilder::visit(ast::StructDecl &Struct) {
  std::vector<llvm::Type *> Args;

  for (auto Field : Struct.getFieldList()) {
    auto Ident = Field->getType()->getIdentifier();
    Args.push_back(Module->getType(Ident)->getIR());
  }

  Struct.getIR()->setBody(Args);
  return nullptr;
}

Value *IRBuilder::visit(ast::EnumDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::UnionDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::TupleDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::RangeDecl &) { return nullptr; }

Value *IRBuilder::visit(ast::TypeDef &Type) {
  return Type.getTypeDecl()->accept(*this);
}

} // namespace north::targets
