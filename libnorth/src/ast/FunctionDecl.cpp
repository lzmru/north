//===--- AST/FunctionDecl.cpp -----------------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/AST.h"
#include "Type/Module.h"
#include "Type/TypeInference.h"

#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/Module.h>


namespace north::ast {

ast::FunctionDecl *GenericFunctionDecl::instantiate(ast::CallExpr *Callee, type::Module *Mod) {
  assert(this->hasGenerics() && "Cannot instantiate non-generic function");
  auto InstantiatedFn = new ast::FunctionDecl(*this);

  for (size_t I = 0; I < InstantiatedFn->getArgumentList().size(); ++I) {
    // TODO: handling errors
    auto FnArg = InstantiatedFn->getArg(I);
    if (auto T = InstantiatedFn->containsGeneric(FnArg->getType()->getIdentifier()); T != -1) {
      auto *Arg = Callee->getArg(I);
      auto *ArgType = inferExprType(Arg->Arg, Mod, Mod->getGlobalScope());
      InstantiatedFn->instantiateGeneric(T, ArgType);
      FnArg->setIRType(ArgType->getIR());
      FnArg->setType(ArgType->getDecl());
    }
  }

  if (auto RetType = InstantiatedFn->getTypeDecl()) {
    if (auto T = InstantiatedFn->containsGeneric(RetType->getIdentifier()); T != -1) {
      auto Generic = InstantiatedFn->getGeneric(T).Type;
      InstantiatedFn->setTypeDecl(Generic->getDecl());
      InstantiatedFn->setTypeIR(Generic->getIR());
    }
  }
  
  this->InstantinatedFunctions.push_back(InstantiatedFn);
  
  return InstantiatedFn;
}
  
namespace {

llvm::GlobalValue::LinkageTypes getLinkageType(north::ast::FunctionDecl *Fn) {
  using LT = llvm::GlobalValue::LinkageTypes;
  return Fn->getIdentifier().front() == '_' ? LT::InternalLinkage
                                            : LT::ExternalLinkage;
}

}
  
void FunctionDecl::createIR(type::Module *Module) {
  llvm::FunctionType *FnType = nullptr;
  llvm::Type *ResultType = nullptr;
  
  if (auto ReturnType = this->getTypeIR()) {
    ResultType = ReturnType;
  } else if (auto ReturnType = this->getTypeDecl()) {
    ResultType = Module->getType(ReturnType->getIdentifier())->getIR();
    if (ReturnType->isPtr())
      ResultType = ResultType->getPointerTo(0);
  } else {
    ResultType = type::Type::Void->getIR();
  }
  
  std::vector<llvm::Type *> ArgList;
  if (this->hasArgs()) {
    auto Args = this->getArgumentList();
    ArgList.reserve(Args.size());

    for (auto Arg : Args) {
      if (auto ArgType = Arg->getIRType()) {
        ArgList.push_back(ArgType);
      } else {
        auto Argument = Module->getType(Arg->getType()->getIdentifier())->getIR();
        ArgList.push_back(
            Arg->getType()->isPtr()
              ? Argument->getPointerTo(0)
              : Argument
              );
      }
    }

    FnType = llvm::FunctionType::get(ResultType, ArgList, this->isVarArg());
  } else {
    FnType = llvm::FunctionType::get(ResultType, this->isVarArg());
  }

  auto IR = llvm::Function::Create(FnType, getLinkageType(this), this->getIdentifier(), *Module);

  for (auto &IrArg : IR->args()) {
    auto AstArg = this->getArg(IrArg.getArgNo());
    AstArg->setIRType(IrArg.getType());
    AstArg->setIRValue(&IrArg);
  }

  this->IR = IR;
}

} // namespace north::ast
