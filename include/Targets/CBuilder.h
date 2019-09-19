//===--- IR/IRBuilder.h - Transformation AST to LLVM IR ---------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_TARGETS_CBUILDER_H
#define NORTH_TARGETS_CBUILDER_H

#include "Targets/BuilderBase.h"

namespace north::targets {

class CBuilder : public ast::Visitor, BuilderBase {
  std::unique_ptr<north::type::Module> Module;
  type::Scope *CurrentScope;
  ast::FunctionDecl *CurrentFn;

public:
  explicit CBuilder(north::type::Module *Module)
      : BuilderBase(Module), CurrentScope(Module->getGlobalScope()), CurrentFn(nullptr) {}

  AST_WALKER_METHODS
};

} // namespace north::targets

#endif // NORTH_TARGETS_CBUILDER_H
