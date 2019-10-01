//===--- Type/Type.cpp - North language type representation -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Type/Type.h"
#include "Targets/IRBuilder.h"

#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/Twine.h>
#include <llvm/Support/raw_ostream.h>

namespace north::type {

namespace {

template <bool IsPointer = false, typename T> Type *createPrimitive(T IRType) {
  if constexpr (IsPointer)
    return new Type(IRType(targets::IRBuilder::getContext())->getPointerTo(0));
  return new Type(IRType(targets::IRBuilder::getContext()));
}

llvm::Type *createStructIR(ast::GenericDecl *Decl, Module *M) {
  auto Struct = static_cast<ast::TypeDef *>(Decl)->getTypeDecl();
  auto IR = llvm::StructType::create(targets::IRBuilder::getContext(),
                                     Decl->getIdentifier());
  static_cast<ast::StructDecl *>(Struct)->setIR(IR);
  return IR;
}

llvm::Type *createEnumIR(ast::GenericDecl *Decl, Module *M) {
  auto Enum = static_cast<ast::EnumDecl *>(Decl);

  uint64_t I = 0;
  for (auto Member : Enum->getMemberList()) {
    Enum->addValue(Member.getTokenInfo().toString(),
                   llvm::ConstantInt::get(Type::Int32->toIR(M), ++I));
  }

  return Type::Int32->toIR(M); // TODO: typed enum
}

llvm::Type *createIR(ast::GenericDecl *Decl, Module *M) {
  ast::TypeDef *TypeDef;
  llvm::Type *Result = nullptr;

  switch (Decl->getKind()) {
  case ast::AST_TypeDef:
    TypeDef = static_cast<ast::TypeDef *>(Decl);

    switch (TypeDef->getTypeDecl()->getKind()) {
    case ast::AST_StructDecl:
      return createStructIR(TypeDef, M);

    case ast::AST_AliasDecl:
      Result = M->getType(TypeDef->getTypeDecl()->getIdentifier())->toIR(M);
      if (Decl->isPtr())
        Result = Result->getPointerTo(0);
      return Result;

    case ast::AST_EnumDecl:
      return createEnumIR(TypeDef->getTypeDecl(), M);

    default:
      break;
    }

  default:
    llvm_unreachable("Invalid type declaration");
  }
}

} // namespace

using IR = llvm::Type;

Type *Type::Void = createPrimitive(IR::getVoidTy);
Type *Type::Int = createPrimitive(IR::getInt32Ty);
Type *Type::Int8 = createPrimitive(IR::getInt8Ty);
Type *Type::Int16 = createPrimitive(IR::getInt16Ty);
Type *Type::Int32 = createPrimitive(IR::getInt32Ty);
Type *Type::Int64 = createPrimitive(IR::getInt64Ty);
Type *Type::Int128 = createPrimitive(IR::getInt128Ty);
Type *Type::Float = createPrimitive(IR::getFloatTy);
Type *Type::Double = createPrimitive(IR::getDoubleTy);
Type *Type::String = createPrimitive<true>(IR::getInt8Ty);
Type *Type::Char = createPrimitive(IR::getInt8Ty);

llvm::Type *Type::toIR(Module *Mod) {
  if (!IRValue) {
    llvm::outs() << "yep\n";
    IRValue = createIR(Decl.get(), Mod);
  }
  llvm::outs() << "toIr(): " << IRValue << '\n';
  return IRValue;
}

} // namespace north::type
