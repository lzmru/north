//===--- Type/Scope.h - North language scope declaration --------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_TYPE_SCOPE_H
#define LIBNORTH_TYPE_SCOPE_H

#include "Type/Module.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringMap.h>

namespace north::ast {

class GenericDecl;
class VarDecl;

} // namespace north::ast

namespace north::type {

class Scope {
  Scope *Parent;
  llvm::StringMap<north::ast::VarDecl *> Vars;
  uint8_t IndentLevel;
  type::Module *Owner;

public:
  explicit Scope(type::Module *Owner)
      : Parent(nullptr), IndentLevel(0), Owner(Owner) {}
  explicit Scope(Scope *Parent, type::Module *Owner)
      : Parent(Parent), IndentLevel(Parent->getIndentLevel() + 1),
        Owner(Owner) {}

  Scope *getParent() { return Parent; }
  void addElement(north::ast::VarDecl *Var);
  uint8_t getIndentLevel() const { return IndentLevel; }
  north::ast::VarDecl *lookup(llvm::StringRef Name);

private:
  north::ast::VarDecl *lookupParentScopes(llvm::StringRef Name);
};

} // namespace north::type

#endif // LIBNORTH_TYPE_SCOPE_H
