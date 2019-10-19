//===--- Type/Module.cpp - North module representation ----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Type/Module.h"
#include "Type/Scope.h"
#include "Type/Type.h"

#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/FunctionImport.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/SourceMgr.h>

namespace north::type {

using namespace llvm;
using namespace sys::path;

Module::Module(llvm::StringRef ModuleID, llvm::LLVMContext &C, llvm::SourceMgr &SourceMgr)
    : llvm::Module(stem(ModuleID), C), GlobalScope(new Scope(this)), SourceManager(SourceMgr) {

  setSourceFileName(ModuleID);

  TypeList.try_emplace("void",   Type::Void  );
  TypeList.try_emplace("i8",     Type::Int8  );
  TypeList.try_emplace("i16",    Type::Int16 );
  TypeList.try_emplace("i32",    Type::Int32 );
  TypeList.try_emplace("i64",    Type::Int64 );
  TypeList.try_emplace("float",  Type::Float );
  TypeList.try_emplace("double", Type::Double);
  TypeList.try_emplace("char",   Type::Char  );
}

Type *Module::getType(llvm::StringRef Name) const {
  auto res = TypeList.find(Name);
  if (res != TypeList.end())
    return res->second;

  auto Range = llvm::SMRange(llvm::SMLoc(), llvm::SMLoc());
  SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
      "The type '" + Name + "' is undefined", Range);

  return nullptr;
}

Type *Module::getTypeOrNull(llvm::StringRef Name) const {
  auto res = TypeList.find(Name);
  if (res != TypeList.end())
    return res->second;
  return nullptr;
}

Module::InterfaceDecl *Module::getInterface(llvm::StringRef Name) const {
  auto res = InterfaceList.find(Name);
  if (res != InterfaceList.end())
    return res->second;

  auto Range = llvm::SMRange(llvm::SMLoc(), llvm::SMLoc());
  SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
      "The interface '" + Name + "' is undefined", Range);

  return nullptr;
}

ast::FunctionDecl *Module::getFn(ast::CallExpr &Callee, Scope *S) {
  auto Ident = Callee.getIdentifier();

  if (Ident->getSize() == 1) {
    // TODO: check named elements
    auto Found = FunctionList.find(Ident->getPart(0));
    if (Found == FunctionList.end())
      return nullptr;

    return Found->second;
  }
  return nullptr;

/*  } else {
    if (auto Var = S->lookup(Ident->getPart(0))) {
      for (auto &Fn : llvm::Module::getFunctionList()) {
        if (!Fn.arg_size())
          break;

        auto ArgTy = Fn.arg_begin()->getType();
        if (ArgTy->isPointerTy() &&
            ArgTy->getPointerElementType()->isStructTy())
          ArgTy = ArgTy->getPointerElementType();

        auto T = Var->getIRType();
        if (T->isArrayTy())
          T = T->getArrayElementType()->getPointerTo(0);

        if (ArgTy == T && Fn.getName() == Ident->getPart(1)) {
          Callee.insertArgument(new ast::LiteralExpr(
              TokenInfo{Var->getPosition(), Token::Identifier}));
          Ident->removeFirst();

          return &Fn;
        }
      }
    }

    return nullptr;
  }*/
}

void Module::addType(north::ast::GenericDecl *TypeDecl) {
  auto Type = new type::Type(TypeDecl, this);
  
  if (!TypeList.try_emplace(TypeDecl->getIdentifier(), Type).second) {
    auto Range = llvm::SMRange(llvm::SMLoc(), llvm::SMLoc());
    SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
        "Duplicate definition of type '" + TypeDecl->getIdentifier() + "'", Range);
  }
}

void Module::addInterface(north::ast::InterfaceDecl *Interface) {
  if (!InterfaceList.try_emplace(Interface->getIdentifier(), Interface).second) {
    auto Range = llvm::SMRange(llvm::SMLoc(), llvm::SMLoc());
    SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
        "Duplicate definition of interface '" +  Interface->getIdentifier() + "'", Range);
  }
}

void Module::addFunction(north::ast::FunctionDecl *Fn) {
  // TODO: overloading
  if (!FunctionList.try_emplace(Fn->getIdentifier(), Fn).second) {
    auto Id = Fn->getIdentifier();
    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Id.data()),
        llvm::SMLoc::getFromPointer(Id.data() + Id.size()));
    SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                               "duplicate definition of function '" +  Id + "'", Range);
  }

  if (Fn->hasGenerics()) {
    this->hasGenericDeclarations = true;
    return;
  }

  Fn->createIR(this);
}

void Module::addImport(north::ast::OpenStmt *Import) {
  ImportList.push_back(Import->getModuleName());
}
  
void Module::checkCall(ast::CallExpr *Callee) {
  assert(Callee);
  
  auto Fn = this->getFn(*Callee, nullptr); // FIXME
  
  if (!Fn) {
    auto Pos = Callee->getPosition();

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                               "unknown function referenced", Range);
  }
  
  if (this->hasGenericDeclarations && Fn->hasGenerics()) {
    Fn = static_cast<ast::GenericFunctionDecl *>(Fn)->instantiate(Callee, this);
    Fn->createIR(this);
  }
  
  Callee->setCallableFn(Fn, this);
}

} // namespace north::type
