//===--- IR/IRBuilder.h - Transformation AST to LLVM IR ---------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_TARGETS_BUILDER_H
#define LIBNORTH_TARGETS_BUILDER_H

#include "AST/Visitor.h"
#include "Type/Module.h"
#include "Type/Scope.h"

#include <llvm/ADT/StringMap.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace north::targets {


class BuilderBase {
protected:
  std::unique_ptr<north::type::Module> Module;
  llvm::SourceMgr& SourceManager;

public:
  explicit BuilderBase(north::type::Module *Module, llvm::SourceMgr& SourceMgr)
      : Module(Module), SourceManager(SourceMgr) { assert(Module != nullptr); }

  llvm::Module *getModule() { return Module.get(); }
};

} // namespace north::targets

#endif // LIBNORTH_TARGETS_BUILDER_H
