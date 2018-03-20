//===--- IR/IRBuilder.h - Transformation AST to LLVM IR ---------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_IR_BUILDER_H
#define NORTH_IR_BUILDER_H

#include "AST/Visitor.h"
#include "Type/Module.h"
#include "Type/Scope.h"

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace north::ir {

class IRBuilder : public ast::Visitor {
  llvm::IRBuilder<> Builder;
  std::unique_ptr<north::type::Module> Module;
  type::Scope *CurrentScope;
  ast::FunctionDecl *CurrentFn;

  static llvm::LLVMContext Context;

public:
  explicit IRBuilder(north::type::Module *Module)
      : Builder(Context), Module(Module),
        CurrentScope(Module->getGlobalScope()), CurrentFn(nullptr) {}

  static llvm::LLVMContext &getContext() { return Context; }
  llvm::Module *getModule() { return Module.get(); }

  AST_WALKER_METHODS

private:
  type::Type *getTypeFromIdent(ast::Node *);
};

} // namespace north::ir

#endif // NORTH_IR_BUILDER_H
