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
llvm::Value *CBuilder::visit(ast::LiteralExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::RangeExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::CallExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::ArrayIndexExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::QualifiedIdentifierExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::IfExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::ForExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::WhileExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::AssignExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::StructInitExpr &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::ArrayExpr &) { return nullptr; }

} // namespace north::targets