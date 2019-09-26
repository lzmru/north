//===--- IR/DeclBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Targets/CBuilder.h"

namespace north::targets {

llvm::Value *CBuilder::visit(ast::UnaryExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::BinaryExpr &) { return nullptr; }

llvm::Value *CBuilder::visit(ast::LiteralExpr &Literal) {
  auto Token = Literal.getTokenInfo();
  switch (Token.Type) {
  case Token::String:
    outs << "\"" << Token.toString() << "\"";
    return nullptr;

  default:
    return nullptr;
  }
}

llvm::Value *CBuilder::visit(ast::RangeExpr &) { return nullptr; }

llvm::Value *CBuilder::visit(ast::CallExpr &Callee) {
  Callee.getIdentifier()->accept(*this);
  outs << "(";

  const auto& ArgList = Callee.getArgumentList();
  for (int I = 0; I < ArgList.size(); ++I) {
    ArgList[I]->Arg->accept(*this);
    outs << (I+1 > ArgList.size() ? ", " : "");
  }

  outs << ");\n";

  return nullptr;
}

llvm::Value *CBuilder::visit(ast::ArrayIndexExpr &) { return nullptr; }

llvm::Value *CBuilder::visit(ast::QualifiedIdentifierExpr &Identifier) {
  outs << Identifier.getIdentifier()[0].toString();

  return nullptr;
}

llvm::Value *CBuilder::visit(ast::IfExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::ForExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::WhileExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::AssignExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::StructInitExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::ArrayExpr &) { return nullptr; }

} // namespace north::targets