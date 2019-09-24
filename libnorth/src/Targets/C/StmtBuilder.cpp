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

llvm::Value *CBuilder::visit(ast::OpenStmt &) {

  return nullptr;
}

llvm::Value *CBuilder::visit(ast::BlockStmt &Block) {
  outs << "{\n";

  if (auto Body = Block.getBody()) {
    for (auto I = Body->begin(), E = Body->end(); I != E; ++I)
      I->accept(*this);
  }

  outs << "\n}\n";

  return nullptr;
}

llvm::Value *CBuilder::visit(ast::ReturnStmt &Return) {
  outs << "return ";
  Return.getReturnExpr()->accept(*this);

  return nullptr;
}

} // namespace north::targets