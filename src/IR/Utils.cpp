//===--- IR/Utils.cpp - Transformation AST to LLVM IR -----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Diagnostic.h"
#include "IR/IRBuilder.h"
#include "Type/Type.h"

namespace north::ir {

using namespace llvm;

type::Type *IRBuilder::getTypeFromIdent(ast::Node *Ident) {
  if (auto Literal = dyn_cast<ast::LiteralExpr>(Ident)) {
    if (auto Type = Module->getTypeOrNull(Literal->getTokenInfo().toString()))
      return Type;
  }

  Diagnostic(Module->getModuleIdentifier()).semanticError("unknown symbol");
}

} // namespace north::ir