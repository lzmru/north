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
  
#define PRIMITIVE(P) \
  new Type(llvm::Type::get ## P ## Ty(targets::IRBuilder::getContext()))

Type *Type::Void   = PRIMITIVE(Void  );
Type *Type::Int8   = PRIMITIVE(Int8  );
Type *Type::Int16  = PRIMITIVE(Int16 );
Type *Type::Int32  = PRIMITIVE(Int32 );
Type *Type::Int64  = PRIMITIVE(Int64 );
Type *Type::Float  = PRIMITIVE(Float );
Type *Type::Double = PRIMITIVE(Double);
Type *Type::Char   = PRIMITIVE(Int8  );

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

llvm::Type *Type::getIR() {
  if (!IRType)
    IRType = createIR(Decl, Mod);
  return IRType;
}
  
void Type::setIR(llvm::Type *T) {
  assert(T && "IR type must not be null");
  IRType = T;
}
  
bool Type::operator==(const Type &RHS) const {
  bool A = true, B = true;
  
  if (this->Decl && RHS.Decl)
    A = this->Decl->getIdentifier() == RHS.Decl->getIdentifier();
    
  // TODO: More precise type checking
  if (this->IRType && RHS.IRType)
    B = this->IRType == RHS.IRType;
  
  return A && B;
}
  
bool Type::operator!=(const Type &RHS) const {
  return !(*this == RHS);
}

} // namespace north::type
