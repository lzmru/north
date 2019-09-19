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

llvm::Value *CBuilder::visit(ast::OpenStmt &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::BlockStmt &) { return nullptr; }
llvm::Value *CBuilder::visit(ast::ReturnStmt &) { return nullptr; }

} // namespace north::targets