//===--- IR/DeclBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Diagnostic.h"
#include "IR/IRBuilder.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>

#define M Module.get()

namespace north::ir {

using namespace llvm;

Value *IRBuilder::visit(ast::FunctionDecl &Fn) {
  if (!Fn.getBlockStmt())
    return nullptr;
  auto BB = BasicBlock::Create(Context, "entry", Fn.getIRValue());
  Builder.SetInsertPoint(BB);
  // inferFunctionType(Fn, M);
  CurrentFn = &Fn;
  return Fn.getBlockStmt()->accept(*this);
}

Value *IRBuilder::visit(ast::InterfaceDecl &) { return nullptr; }

Value *IRBuilder::visit(ast::VarDecl &Var) {
  llvm::Type *Type = nullptr;

  if (auto TypeDecl = Var.getType()) {
    Type = Module->getType(Var.getType()->getIdentifier())->toIR(M);
    if (TypeDecl->isPtr())
      Type = Type->getPointerTo(0);
  } else {
    Type = inferVarType(Var, M, CurrentScope)->toIR(M);
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
    Args.push_back(Module->getType(Ident)->toIR(M));
  }

  Struct.getIR()->setBody(Args);
  return nullptr;
}

Value *IRBuilder::visit(ast::EnumDecl &Enum) { return nullptr; }
Value *IRBuilder::visit(ast::UnionDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::TupleDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::RangeDecl &) { return nullptr; }

Value *IRBuilder::visit(ast::TypeDef &Type) {
  return Type.getTypeDecl()->accept(*this);
}

} // namespace north::ir