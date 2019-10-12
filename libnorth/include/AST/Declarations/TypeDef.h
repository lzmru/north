#ifndef LIBNORTH_AST_DECLARATIONS_TYPEDEF_H
#define LIBNORTH_AST_DECLARATIONS_TYPEDEF_H

namespace north::ast {

class TypeDef : public GenericDecl {
  GenericDecl *Type;

public:
  explicit TypeDef(const TokenInfo &TkInfo, TypeDef *Type = nullptr)
      : GenericDecl(TkInfo.Pos, AST_TypeDef, TkInfo.toString()), Type(Type) {}

  GenericDecl *getTypeDecl() { return Type; }
  void setTypeDecl(GenericDecl *NewType) { Type = NewType; }

  AST_NODE(TypeDef)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_TYPEDEF_H
