//===--- IR/StmtBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Diagnostic.h"
#include "Grammar/Parser.h"
#include "Targets/IRBuilder.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"

#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>

#define M Module.get()

namespace north::targets {

using namespace llvm;

llvm::LLVMContext IRBuilder::Context;

Value *IRBuilder::visit(ast::OpenStmt &O) {
  std::string Path = sys::path::parent_path(Module->getSourceFileName());
  Path += "/";
  Path += O.getModuleName();
  Path += ".north";

  north::Parser TheParser(&*Module, Path.c_str());
  auto NewMod = TheParser.parse();

  auto AST = NewMod->getAST();
  for (auto I = AST->begin(), E = AST->end(); I != E; ++I)
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

  if (auto TD = CurrentFn->getTypeDecl()) {
    auto IRType = type::inferFunctionType(*CurrentFn, M, CurrentScope)->toIR(M);
    auto NorthType = Module->getType(TD->getIdentifier())->toIR(M);
    if (IRType != NorthType)
      Diagnostic(Module->getModuleIdentifier())
          .semanticError("return value type of `" + CurrentFn->getIdentifier() +
                         "` does't match the function type");
  } else {
    Builder.CreateRetVoid();
  }

  CurrentScope = Scope.getParent();
  return Result;
}

Value *IRBuilder::visit(ast::ReturnStmt &Return) {
  if (auto Expr = Return.getReturnExpr()) {
    GetVal = true;
    return Builder.CreateRet(Expr->accept(*this));
  }
  return Builder.CreateRetVoid();
}

} // namespace north::targets
