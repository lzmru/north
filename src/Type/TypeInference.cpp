//===--- Type/TypeInference.cpp - Compute a type of every expr --*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Type/TypeInference.h"
#include "AST/AST.h"
#include "Diagnostic.h"
#include "Type/Module.h"
#include "Type/Type.h"
#include <llvm/ADT/StringMap.h>
#include <llvm/Support/raw_ostream.h>

namespace north::type {

namespace detail {

class NamedValue : public llvm::Value {
public:
  explicit NamedValue(llvm::StringRef Name) : Name(Name), Value(nullptr, 0) {}

  llvm::StringRef Name;
};

class TypedValue : public llvm::Value {
public:
  explicit TypedValue(llvm::Type *Type) : Type(Type), Value(nullptr, 1) {}

  llvm::Type *Type;
};

llvm::Value *InferenceVisitor::visit(ast::FunctionDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::InterfaceDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::VarDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::AliasDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::StructDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::EnumDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::UnionDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::TupleDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::RangeDecl &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::TypeDef &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::UnaryExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::BinaryExpr &Binary) {
  return Binary.getRHS()->accept(*this);
}

llvm::Value *InferenceVisitor::visit(ast::LiteralExpr &Literal) {
  switch (Literal.getTokenInfo().Type) {
  case Token::Char:
    return new NamedValue("char");
  case Token::Int:
    return new NamedValue("int");
  case Token::String:
    return new NamedValue("string");
  }
}

llvm::Value *InferenceVisitor::visit(ast::RangeExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::CallExpr &Callee) {
  return new TypedValue(Mod->getFunction(Callee.getIdentifier())
                            ->getFunctionType()
                            ->getReturnType());
}

llvm::Value *InferenceVisitor::visit(ast::IfExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::ForExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::WhileExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::AssignExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::StructInitExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::ArrayExpr &Array) {
  return new TypedValue(llvm::ArrayType::get(
      type::inferExprType(Array.getValue(0), Mod)->toIR(Mod), Array.getCap()));
}

llvm::Value *InferenceVisitor::visit(ast::OpenStmt &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::BlockStmt &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::ReturnStmt &Return) {
  return Return.getReturnExpr()->accept(*this);
}

} // namespace detail

Type *inferFunctionType(ast::FunctionDecl &Fn, Module *Mod) {
  llvm::Value *Val = nullptr;
  auto Visitor = detail::InferenceVisitor(Mod);
  auto Body = Fn.getBlockStmt()->getBody();

  for (auto I = Body->begin(), E = Body->end(); I != E; ++I) {
    if (auto Return = llvm::dyn_cast<ast::ReturnStmt>(I))
      Val = I->accept(Visitor);
  }

  return Mod->getType(static_cast<detail::NamedValue *>(Val)->Name);
}

Type *inferVarType(ast::VarDecl &Var, Module *Mod) {
  auto Visitor = detail::InferenceVisitor(Mod);
  auto Type = Var.getValue()->accept(Visitor);

  if (Type->getValueID() == 0)
    return Mod->getType(static_cast<detail::NamedValue *>(Type)->Name);
  else
    return new type::Type(
        static_cast<detail::TypedValue *>(Type)->Type); // FIXME
}

Type *inferExprType(ast::Node *Expr, Module *Mod) {
  auto Visitor = detail::InferenceVisitor(Mod);
  auto Type = Expr->accept(Visitor);

  if (Type->getValueID() == 0)
    return Mod->getType(static_cast<detail::NamedValue *>(Type)->Name);
  else
    return new type::Type(
        static_cast<detail::TypedValue *>(Type)->Type); // FIXME
}

} // namespace north::type