//===--- Type/TypeInference.cpp - Compute a type of every expr --*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_TYPE_TYPEINFERENCE_H
#define LIBNORTH_TYPE_TYPEINFERENCE_H

#include "AST/Visitor.h"
#include "Type.h"

namespace north::type {

class Module;
class Scope;

Type *inferFunctionType(ast::FunctionDecl &, Module *, Scope *);
Type *inferVarType(ast::VarDecl &, Module *, Scope *);
Type *inferExprType(ast::Node *, Module *, Scope *);

namespace detail {

class InferenceVisitor : public ast::Visitor {
  Module *Mod;
  Scope *CurrentScope;

public:
  explicit InferenceVisitor(Module *Mod, Scope *Scope)
      : Mod(Mod), CurrentScope(Scope){};
  AST_WALKER_METHODS
};

} // namespace detail

} // namespace north::type

#endif // LIBNORTH_TYPE_TYPEINFERENCE_H
