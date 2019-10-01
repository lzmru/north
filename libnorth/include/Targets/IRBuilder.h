//===--- IR/IRBuilder.h - Transformation AST to LLVM IR ---------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_IR_BUILDER_H
#define LIBNORTH_IR_BUILDER_H

#include "BuilderBase.h"

namespace north::targets {

class IRBuilder : public ast::Visitor, BuilderBase {
  llvm::IRBuilder<> Builder;
  type::Scope *CurrentScope;
  ast::FunctionDecl *CurrentFn;

  bool GetVal = false;
  bool LoadArg = false;

  static llvm::LLVMContext Context;

public:
  explicit IRBuilder(north::type::Module *Module, llvm::SourceMgr& SourceManager)
      : BuilderBase(Module, SourceManager), Builder(Context),
        CurrentScope(Module->getGlobalScope()), CurrentFn(nullptr) {}

  static llvm::LLVMContext &getContext() { return Context; }

  AST_WALKER_METHODS

private:
  type::Type *getTypeFromIdent(ast::Node *);
  llvm::Value *cmpWithTrue(llvm::Value *);
  llvm::Value *getStructField(ast::Node *, llvm::Value *,
                              ast::QualifiedIdentifierExpr &);
  llvm::Constant *createEscapedString(ast::LiteralExpr &);
};

} // namespace north::targets

#endif // LIBNORTH_IR_BUILDER_H
