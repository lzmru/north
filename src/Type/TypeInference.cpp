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
#include "Type/Scope.h"
#include "Type/Type.h"
#include <llvm/ADT/StringMap.h>
#include <llvm/Support/raw_ostream.h>

namespace north::type {

namespace detail {

class NamedValue : public llvm::Value {
public:
  explicit NamedValue(llvm::StringRef Name) : Value(nullptr, 0), Name(Name) {}

  llvm::StringRef Name;
};

class TypedValue : public llvm::Value {
public:
  explicit TypedValue(llvm::Type *Type) : Value(nullptr, 1), Type(Type) {}
  explicit TypedValue(llvm::Value *Val)
      : Value(nullptr, 1), Type(Val->getType()) {}

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
  auto L = Literal.getTokenInfo();
  switch (L.Type) {
  case Token::Char:
    return new NamedValue("char");
  case Token::Int:
    return new NamedValue("int");
  case Token::String:
    return new NamedValue("string");
  default:
    break;
  }

  if (auto Var = CurrentScope->lookup(L.toString()))
    return new TypedValue(Var->getIRType());
  if (auto Type = Mod->getTypeOrNull(L.toString()))
    return new TypedValue(Type->toIR(Mod));

  Diagnostic(Mod->getModuleIdentifier())
      .semanticError("unknown symbol `" + L.toString() + "`");
  return nullptr;
}

llvm::Value *InferenceVisitor::visit(ast::RangeExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::CallExpr &Callee) {
  return new TypedValue(Mod->getFunction(Callee.getIdentifier())
                            ->getFunctionType()
                            ->getReturnType());
}

llvm::Value *InferenceVisitor::visit(ast::ArrayIndexExpr &Idx) {
  return new TypedValue(Idx.getIdentifier()->accept(*this));
}

llvm::Value *InferenceVisitor::visit(ast::QualifiedIdentifierExpr &Ident) {
  auto I = Ident.getPart(0);
  if (auto Var = CurrentScope->lookup(I))
    return new TypedValue(Var->getIRType());
  if (auto Type = Mod->getTypeOrNull(I))
    return new TypedValue(Type->toIR(Mod));

  return nullptr;
}

llvm::Value *InferenceVisitor::visit(ast::IfExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::ForExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::WhileExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::AssignExpr &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::StructInitExpr &SI) {
  return SI.getIdentifier()->accept(*this);
}

llvm::Value *InferenceVisitor::visit(ast::ArrayExpr &Array) {
  auto ArrTy =
      type::inferExprType(Array.getValue(0), Mod, CurrentScope)->toIR(Mod);
  return new TypedValue(llvm::ArrayType::get(ArrTy, Array.getCap()));
}

llvm::Value *InferenceVisitor::visit(ast::OpenStmt &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::BlockStmt &) { return nullptr; }

llvm::Value *InferenceVisitor::visit(ast::ReturnStmt &Return) {
  return Return.getReturnExpr()->accept(*this);
}

} // namespace detail

Type *inferFunctionType(ast::FunctionDecl &Fn, Module *Mod,
                        Scope *CurrentScope) {
  llvm::Value *Type = nullptr;
  auto Visitor = detail::InferenceVisitor(Mod, CurrentScope);
  auto Body = Fn.getBlockStmt()->getBody();

  for (auto I = Body->begin(), E = Body->end(); I != E; ++I) {
    if (auto Return = llvm::dyn_cast<ast::ReturnStmt>(I))
      Type = I->accept(Visitor);
  }

  if (Type->getValueID() == 0)
    return Mod->getType(static_cast<detail::NamedValue *>(Type)->Name);
  else
    return new type::Type(
        static_cast<detail::TypedValue *>(Type)->Type); // FIXME
}

Type *inferVarType(ast::VarDecl &Var, Module *Mod, Scope *CurrentScope) {
  auto Visitor = detail::InferenceVisitor(Mod, CurrentScope);
  auto Type = Var.getValue()->accept(Visitor);

  if (Type->getValueID() == 0)
    return Mod->getType(static_cast<detail::NamedValue *>(Type)->Name);
  else
    return new type::Type(
        static_cast<detail::TypedValue *>(Type)->Type); // FIXME
}

Type *inferExprType(ast::Node *Expr, Module *Mod, Scope *CurrentScope) {
  auto Visitor = detail::InferenceVisitor(Mod, CurrentScope);
  auto Type = Expr->accept(Visitor);

  if (Type->getValueID() == 0)
    return Mod->getType(static_cast<detail::NamedValue *>(Type)->Name);
  else
    return new type::Type(
        static_cast<detail::TypedValue *>(Type)->Type); // FIXME
}

} // namespace north::type
