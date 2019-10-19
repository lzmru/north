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

llvm::Type *createStructIR(ast::GenericDecl *Decl) {
  auto Struct = static_cast<ast::TypeDef *>(Decl)->getTypeDecl();
  auto IR = llvm::StructType::create(targets::IRBuilder::getContext(),
                                     Decl->getIdentifier());
  static_cast<ast::StructDecl *>(Struct)->setIR(IR);
  return IR;
}

llvm::Type *createEnumIR(ast::GenericDecl *Decl) {
  auto Enum = static_cast<ast::EnumDecl *>(Decl);

  uint64_t I = 0;
  for (auto Member : Enum->getMemberList()) {
    Enum->addValue(Member.getTokenInfo().toString(),
                   llvm::ConstantInt::get(Type::Int32->getIR(), ++I));
  }

  return Type::Int32->getIR(); // TODO: typed enum
}

llvm::Type *createIR(ast::GenericDecl *Decl, Module *M) {
  assert(Decl && "Can't generate IR without type decla");

  ast::TypeDef *TypeDef;
  llvm::Type *Result = nullptr;

  switch (Decl->getKind()) {
  case ast::AST_TypeDef:
    TypeDef = static_cast<ast::TypeDef *>(Decl);

    switch (TypeDef->getTypeDecl()->getKind()) {
    case ast::AST_StructDecl:
      return createStructIR(TypeDef);

    case ast::AST_AliasDecl:
      Result = M->getType(TypeDef->getTypeDecl()->getIdentifier())->getIR();
      if (Decl->isPtr())
        Result = Result->getPointerTo(0);
      return Result;

    case ast::AST_EnumDecl:
      return createEnumIR(TypeDef->getTypeDecl());

    default:
      break;
    }

  default:
    llvm_unreachable("Invalid type declaration");
  }
}

} // namespace
  
#define PRIMITIVE(T) new Type(T(targets::IRBuilder::getContext()))

using IR = llvm::Type;

Type *Type::Void   = PRIMITIVE(IR::getVoidTy  );
Type *Type::Int8   = PRIMITIVE(IR::getInt8Ty  );
Type *Type::Int16  = PRIMITIVE(IR::getInt16Ty );
Type *Type::Int32  = PRIMITIVE(IR::getInt32Ty );
Type *Type::Int64  = PRIMITIVE(IR::getInt64Ty );
Type *Type::Float  = PRIMITIVE(IR::getFloatTy );
Type *Type::Double = PRIMITIVE(IR::getDoubleTy);
Type *Type::Char   = PRIMITIVE(IR::getInt8Ty  );

llvm::Type *Type::getIR() {
  if (!IRType)
    IRType = createIR(Decl, Mod);
  return IRType;
}
  
void Type::setIR(llvm::Type *T) {
  assert(T && "IR type must not be null");
  IRType = T;
}

} // namespace north::type
