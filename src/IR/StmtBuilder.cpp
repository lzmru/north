//===--- IR/StmtBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/Dumper.h"
#include "Diagnostic.h"
#include "Grammar/Parser.h"
#include "IR/IRBuilder.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"

#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#define M Module.get()

namespace north::ir {

using namespace llvm;

llvm::LLVMContext IRBuilder::Context;

Value *IRBuilder::visit(ast::OpenStmt &O) {
  std::string Path = sys::path::parent_path(Module->getSourceFileName());
  Path += "/";
  Path += O.getModuleName();
  Path += ".north";

  north::Parser parser(&*Module, Path.c_str());
  auto Module = parser.parse();

  for (auto I = Module->getAST()->begin(), E = Module->getAST()->end(); I != E;
       ++I)
    I->accept(*this);

  return nullptr;
}

Value *IRBuilder::visit(ast::BlockStmt &Block) {
  type::Scope Scope(CurrentScope, M);
  CurrentScope = &Scope;

  for (auto Arg : CurrentFn->getArgumentList())
    CurrentScope->addElement(Arg);

  Value *Result = nullptr;

  if (auto Body = Block.getBody()) {
    for (auto I = Body->begin(), E = Body->end(); I != E; ++I)
      Result = I->accept(*this);
  }

  CurrentScope = Scope.getParent();
  return Result;
}

Value *IRBuilder::visit(ast::ReturnStmt &Return) {
  if (auto Expr = Return.getReturnExpr())
    return Builder.CreateRet(Expr->accept(*this));
  return Builder.CreateRetVoid();
}

} // namespace north::ir
