//===--- Type/Module.cpp - North module representation ----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Type/Module.h"
#include "Diagnostic.h"
#include "IR/IRBuilder.h"
#include "Type/Type.h"
#include <llvm/Support/Path.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

namespace north::type {

using namespace llvm;
using namespace sys::path;

Module::Module(llvm::StringRef ModuleID, llvm::LLVMContext &C)
    : llvm::Module(stem(ModuleID), C), GlobalScope(new Scope(this)) {

  llvm::Module::setSourceFileName(ModuleID);

  TypeList.try_emplace("void", Type::Void);
  TypeList.try_emplace("int", Type::Int);
  TypeList.try_emplace("i8", Type::Int8);
  TypeList.try_emplace("i16", Type::Int16);
  TypeList.try_emplace("i32", Type::Int32);
  TypeList.try_emplace("i64", Type::Int64);
  TypeList.try_emplace("i128", Type::Int128);
  TypeList.try_emplace("float", Type::Float);
  TypeList.try_emplace("double", Type::Double);
  TypeList.try_emplace("string", Type::String);
  TypeList.try_emplace("char", Type::Char);
}

Type *Module::getType(llvm::StringRef Name) const {
  auto res = TypeList.find(Name);
  if (res != TypeList.end())
    return res->second;
  
  Diagnostic(llvm::Module::getSourceFileName())
      .semanticError("The type '" + Name + "' is undefined");
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
  Diagnostic(llvm::Module::getSourceFileName())
      .semanticError("The interface '" + Name + "' is undefined");
  return nullptr;
}

void Module::addType(north::ast::GenericDecl *TypeDecl) {
  if (!TypeList.try_emplace(TypeDecl->getIdentifier(), new Type(TypeDecl, this))
           .second) {
    Diagnostic(llvm::Module::getSourceFileName())
        .semanticError("Duplicate definition of type '" +
                       TypeDecl->getIdentifier() + "'");
  }
}

void Module::addInterface(north::ast::InterfaceDecl *Interface) {
  if (!InterfaceList.try_emplace(Interface->getIdentifier(), Interface)
           .second) {
    Diagnostic(llvm::Module::getSourceFileName())
        .semanticError("Duplicate definition of interface '" +
                       Interface->getIdentifier() + "'");
  }
}

GlobalValue::LinkageTypes getLinkageType(north::ast::FunctionDecl *Fn) {
  using LT = GlobalValue::LinkageTypes;
  return Fn->getIdentifier().front() == '_' ? LT::InternalLinkage
                                            : LT::ExternalLinkage;
}

void Module::addFunction(north::ast::FunctionDecl *Fn) {
  llvm::FunctionType *FnType = nullptr;
  llvm::Type *ResultType = nullptr;

  if (auto ReturnType = Fn->getTypeDecl()) {
    ResultType = getType(ReturnType->getIdentifier())->toIR(this);
    if (Fn->getTypeDecl()->isPtr())
      ResultType = ResultType->getPointerTo(0);
  } else {
    ResultType = Type::Void->toIR(this);
  }

  if (Fn->hasArgs()) {
    std::vector<llvm::Type *> ArgList;
    auto Args = Fn->getArgumentList();
    ArgList.reserve(Args.size());

    for (auto Arg : Args) {
      auto Argument = getType(Arg->getType()->getIdentifier())->toIR(this);
      ArgList.push_back(Arg->getType()->isPtr() ? Argument->getPointerTo(0)
                                                : Argument);
    }

    FnType = FunctionType::get(ResultType, ArgList, Fn->isVarArg());
  } else {
    FnType = FunctionType::get(ResultType, Fn->isVarArg());
  }

  auto IR =
      Function::Create(FnType, getLinkageType(Fn), Fn->getIdentifier(), this);

  for (Argument &IrArg : IR->args()) {
    auto AstArg = Fn->getArg(IrArg.getArgNo());
    AstArg->setIRType(IrArg.getType());
    AstArg->setIRValue(&IrArg);
  }

  Fn->setIRValue(IR);
}

} // namespace north::type
