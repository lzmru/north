//===--- IR/IRBuilder.h - Transformation AST to LLVM IR ---------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_TARGETS_CBUILDER_H
#define LIBNORTH_TARGETS_CBUILDER_H

#include "BuilderBase.h"
#include <llvm/Support/raw_ostream.h>

namespace north::targets {

class CBuilder : public ast::Visitor, BuilderBase {
  std::unique_ptr<north::type::Module> Module;
  type::Scope *CurrentScope;
  ast::FunctionDecl *CurrentFn;

  llvm::raw_fd_ostream outs;
  std::error_code EC;

public:
  explicit CBuilder(north::type::Module *Module)
      : BuilderBase(Module),
      CurrentScope(Module->getGlobalScope()),
      CurrentFn(nullptr),
      outs(Module->getModuleIdentifier() + ".c", EC) {}

  AST_WALKER_METHODS
};

} // namespace north::targets

#endif // LIBNORTH_TARGETS_CBUILDER_H
