//===--- IR/Utils.cpp - Transformation AST to LLVM IR -----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/Dumper.h"
#include "Diagnostic.h"
#include "IR/IRBuilder.h"
#include "Type/Type.h"
#include <llvm/Support/raw_ostream.h>

namespace north::ir {

using namespace llvm;

type::Type *IRBuilder::getTypeFromIdent(ast::Node *Ident) {
  if (auto Literal = dyn_cast<ast::LiteralExpr>(Ident)) {
    if (auto Type = Module->getTypeOrNull(Literal->getTokenInfo().toString()))
      return Type;
  }

  Diagnostic(Module->getModuleIdentifier()).semanticError("unknown symbol");
  return nullptr;
}

Value *IRBuilder::cmpWithTrue(llvm::Value *Val) {
  return Builder.CreateICmpSGE(Val, ConstantInt::get(Val->getType(), 1, false));
}

Value *IRBuilder::getStructField(ast::Node *Expr, Value *IR,
                                 ast::QualifiedIdentifierExpr &Ident) {
  if (auto InitExpr = dyn_cast<ast::StructInitExpr>(Expr)) {
    auto IRVal = IR;

    auto getFieldNumber = [&](StringRef FieldName) -> Constant * {
      auto Struct = InitExpr->getType();
      uint64_t I = 0;
      for (auto F : Struct->getFieldList()) {
        if (F->getIdentifier() == FieldName) {
          InitExpr = static_cast<ast::StructInitExpr *>(InitExpr->getValue(I));
          return ConstantInt::get(IntegerType::getInt32Ty(Context), I);
        }
        ++I;
      }

      Diagnostic(Module->getModuleIdentifier())
          .semanticError("structure " + Struct->getIdentifier() +
                         "doesn't has field `" + FieldName + "`");
    };

    std::vector<Value *> Indicies{
        ConstantInt::get(IntegerType::getInt32Ty(Context), 0)};
    for (auto Part = 1; Part <= Ident.getSize() - 1; ++Part)
      Indicies.push_back(getFieldNumber(Ident.getPart(Part)));

    auto GEP = Builder.CreateInBoundsGEP(IRVal, Indicies);
    return GetVal ? Builder.CreateLoad(GEP) : GEP;
  }

  if (auto InitExpr = dyn_cast<ast::CallExpr>(Expr)) {
    auto IRVal = IR;
    auto Identifier = InitExpr->getIR()->getType()->getStructName();

    auto getFieldNumber = [&](StringRef FieldName) -> Constant * {
      auto TypeDecl = Module.get()->getType(Identifier)->getDecl();
      auto Struct = static_cast<ast::StructDecl *>(
          static_cast<ast::TypeDef *>(TypeDecl)->getTypeDecl());

      uint64_t I = 0;
      for (auto F : Struct->getFieldList()) {
        if (F->getIdentifier() == FieldName) {
          Identifier = Struct->getField(I)->getType()->getIdentifier();
          return ConstantInt::get(IntegerType::getInt32Ty(Context), I);
        }
        ++I;
      }

      Diagnostic(Module->getModuleIdentifier())
          .semanticError("structure " + Struct->getIdentifier() +
                         "doesn't has field `" + FieldName + "`");
    };

    std::vector<Value *> Indicies{
        ConstantInt::get(IntegerType::getInt32Ty(Context), 0)};
    for (auto Part = 1; Part <= Ident.getSize() - 1; ++Part) {
      Indicies.push_back(getFieldNumber(Ident.getPart(Part)));
    }

    auto GEP = Builder.CreateInBoundsGEP(IRVal, Indicies);
    return GetVal ? Builder.CreateLoad(GEP) : GEP;
  }

  llvm_unreachable("struct w/o initializer");
}

} // namespace north::ir
