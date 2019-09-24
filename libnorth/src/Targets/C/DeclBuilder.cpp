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

llvm::Value *CBuilder::visit(ast::FunctionDecl &Fn) {
  outs << (Fn.getTypeDecl() ? Fn.getTypeDecl()->getIdentifier() : "void") << " "
       << Fn.getIdentifier() << "(";

  const auto& ArgList = Fn.getArgumentList();
  for (int I = 0; I < ArgList.size(); ++I) {
    const auto& Arg = ArgList[I];
    outs << Arg->getType()->getIdentifier() << " "
         << Arg->getIdentifier()
         << (I != ArgList.size()-1 ? ", " : "");
  }

  if (Fn.isVarArg())
    outs << (ArgList.size() ? ", " : "") << "...";

  outs << ")\n";

  if (auto Block = Fn.getBlockStmt())
    Block->accept(*this);

  return nullptr;
}

llvm::Value *CBuilder::visit(ast::InterfaceDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::VarDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::AliasDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::StructDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::EnumDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::UnionDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::TupleDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::RangeDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::TypeDef &) { return nullptr; }

} // namespace north::targets