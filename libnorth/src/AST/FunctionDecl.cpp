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
#include <llvm/IR/Module.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/SourceMgr.h>

namespace north::ast {

void GenericFunctionDecl::addCallExpr(ast::CallExpr *Callee) {
  assert(Callee);
  this->Calls.push_back(Callee);
}

FunctionDecl *GenericFunctionDecl::instantiate(ast::CallExpr *Callee,
                                               type::Module *Mod) {
  assert(this->hasGenerics() && "Cannot instantiate non-generic function");

  InstantiatedFn Fn;
  size_t CountOfArgs = this->countOfArgs();

  // Infer generic types
  for (auto &Generic : this->getGenericsList()) {
    for (size_t I = 0; I < CountOfArgs; ++I) {
      if (Generic.Name == this->getArg(I)->getType()->getIdentifier()) {
        Generic.Type =
            inferExprType(Callee->getArg(I)->Arg, Mod, Mod->getGlobalScope());
        Fn.Types.push_back(Generic);
        break;
      }
    }
  }

  // Was all generic types inferred?
  if (Fn.Types.size() < this->countOfGenerics()) {
    auto Args = Callee->getArgumentList();
    auto Start = Args.front()->Arg->getPosition().Offset;
    auto LastArg = Args.back()->Arg->getPosition();
    auto End = LastArg.Offset + LastArg.Length;

    auto Range = llvm::SMRange(llvm::SMLoc::getFromPointer(Start),
                               llvm::SMLoc::getFromPointer(End));

    Mod->getSourceManager().PrintMessage(Range.Start,
                                         llvm::SourceMgr::DiagKind::DK_Error,
                                         "can't infer type", Range);

    return nullptr;
  }

  if (auto AlreadyInstantiated = this->isInstantiatedAlready(Fn.Types))
    return AlreadyInstantiated;

  Fn.Fn = new ast::FunctionDecl(*this);
  // Instantiate arguments
  for (size_t I = 0; I < CountOfArgs; ++I) {
    // TODO: handling errors
    auto FnArg = this->getArg(I);
    if (auto T = this->containsGeneric(FnArg->getType()->getIdentifier());
        T != -1) {
      auto &ArgType = Fn.Types[T].Type;
      Fn.Fn->instantiateGeneric(T, ArgType);
      FnArg->setIRType(ArgType->getIR());
    }
  }

  // Instantiate return type
  if (auto RetType = Fn.Fn->getTypeDecl()) {
    if (auto T = Fn.Fn->containsGeneric(RetType->getIdentifier()); T != -1) {
      auto Generic = Fn.Fn->getGeneric(T).Type;
      Fn.Fn->setTypeIR(Generic->getIR());
    }
  }

  // Cache result
  this->addInstantiatedFunction(Fn);

  return Fn.Fn;
}

FunctionDecl *GenericFunctionDecl::isInstantiatedAlready(
    llvm::ArrayRef<GenericDecl::Generic> TypeList) const {

  for (auto Fn : this->InstantiatedFunctions) {
    if (Fn.Types.size() != TypeList.size())
      continue;

    bool IsInstantiatedAlready = true;

    for (auto &Generic : Fn.Types) {
      IsInstantiatedAlready = true;

      for (auto &IGeneric : TypeList) {
        if (*Generic.Type != *IGeneric.Type) {
          IsInstantiatedAlready = false;
          break;
        }
      }

      if (IsInstantiatedAlready)
        break;
    }

    if (IsInstantiatedAlready)
      return Fn.Fn;
  }

  return nullptr;
}

namespace {

llvm::GlobalValue::LinkageTypes getLinkageType(north::ast::FunctionDecl *Fn) {
  using LT = llvm::GlobalValue::LinkageTypes;
  return Fn->getIdentifier().front() == '_' ? LT::InternalLinkage
                                            : LT::ExternalLinkage;
}

} // namespace

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
        auto Argument =
            Module->getType(Arg->getType()->getIdentifier())->getIR();
        ArgList.push_back(Arg->getType()->isPtr() ? Argument->getPointerTo(0)
                                                  : Argument);
      }
    }

    FnType = llvm::FunctionType::get(ResultType, ArgList, this->isVarArg());
  } else {
    FnType = llvm::FunctionType::get(ResultType, this->isVarArg());
  }

  auto IR = llvm::Function::Create(FnType, getLinkageType(this),
                                   this->getIdentifier(), *Module);

  for (auto &IrArg : IR->args()) {
    auto AstArg = this->getArg(IrArg.getArgNo());
    AstArg->setIRType(IrArg.getType());
    AstArg->setIRValue(&IrArg);
  }

  this->IR = IR;
}

} // namespace north::ast
