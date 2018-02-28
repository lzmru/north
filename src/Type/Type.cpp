//===--- Type/Type.cpp - North language type representation -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Type/Type.h"
#include "IR/IRBuilder.h"

#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/raw_ostream.h>

namespace north::type {

namespace {

template <bool IsPointer = false, typename T> Type *createPrimitive(T IRType) {
  if constexpr (IsPointer)
    return new Type(IRType(ir::IRBuilder::getContext())->getPointerTo(0));
  return new Type(IRType(ir::IRBuilder::getContext()));
}

llvm::Type *createStructIR(ast::GenericDecl *Decl, Module *M) {
  auto Struct = llvm::StructType::create(ir::IRBuilder::getContext(),
                                         Decl->getIdentifier());
  static_cast<ast::StructDecl *>(Decl)->setIR(Struct);
  return Struct;
}

llvm::Type *createIR(ast::GenericDecl *Decl, Module *M) {
  ast::TypeDef *TypeDef;

  switch (Decl->getKind()) {
  case ast::AST_TypeDef:
    TypeDef = static_cast<ast::TypeDef *>(Decl);

    switch (TypeDef->getTypeDecl()->getKind()) {
    case ast::AST_StructDecl:
      return createStructIR(TypeDef->getTypeDecl(), M);

    case ast::AST_AliasDecl:
      // auto Type =
      //    M->getType(TypeDef->getTypeDecl()->getIdentifier())->toIR(M);
      // return llvm::GlobalAlias::create(
      //           Type, 0, llvm::GlobalAlias::LinkageTypes::InternalLinkage,
      //           Decl->getIdentifier(), M)
      //    ->getType();
      return M->getType(TypeDef->getTypeDecl()->getIdentifier())->toIR(M);
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
  if (!IRValue)
    IRValue = createIR(Decl.get(), Mod);
  return IRValue;
}

} // namespace north::type