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

llvm::Value *CBuilder::visit(ast::FunctionDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::InterfaceDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::VarDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::AliasDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::StructDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::EnumDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::UnionDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::TupleDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::RangeDecl &) { return nullptr; }
//llvm::Value *CBuilder::visit(ast::ArrayDecl &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::TypeDef &) { return nullptr; }

} // namespace north::targets