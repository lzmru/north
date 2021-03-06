//===--- Type/Scope.cpp - North language scope declaration ------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Type/Scope.h"
#include "AST/AST.h"
#include "Type/Module.h"

namespace north::type {

void Scope::addElement(north::ast::VarDecl *Var) {
  if (!Vars.try_emplace(Var->getIdentifier(), Var).second) {
    auto Pos = Var->getPosition();

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Owner->getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
        "duplicate definition of variable '" + Var->getIdentifier() + "'", Range);

  }
}

north::ast::VarDecl *Scope::lookupParentScopes(llvm::StringRef Name) {
  auto Res = Vars.find(Name);
  if (Res != Vars.end())
    return Res->second;
  return !Parent ? nullptr : Parent->lookup(Name);
}

north::ast::VarDecl *Scope::lookup(llvm::StringRef Name) {
  if (auto Res = lookupParentScopes(Name))
    return Res;
  // TODO: global variables
  // if (auto Res = Owner->getGlobalVariable(Name))
  //  return Res;
  return nullptr;
}

} // namespace north::type