#ifndef LIBNORTH_AST_DECLARATIONS_VARDECL_H
#define LIBNORTH_AST_DECLARATIONS_VARDECL_H

namespace north::ast {

class VarDecl : public Declaration {
  GenericDecl *Type = nullptr;
  Node *Value  = nullptr;

  llvm::Value *IRValue = nullptr;
  llvm::Type *IRType = nullptr;

  bool IsArg;
  llvm::StringRef NamedArg;

public:
  explicit VarDecl(const TokenInfo &TkInfo, bool Arg = false)
      : Declaration(TkInfo.Pos, AST_VarDecl, TkInfo.toString()), IsArg(Arg), NamedArg(TkInfo.toString()) {}
  ~VarDecl() {}

  GenericDecl *getType() { return Type; }
  void setType(GenericDecl *NewType) { Type = NewType; }

  Node *getValue() { return Value; }
  void setValue(Node *NewValue) { Value = NewValue; }

  llvm::Value *getIRValue() { return IRValue; }
  void setIRValue(llvm::Value *Val) { IRValue = Val; }

  llvm::Type *getIRType() { return IRType; }
  void setIRType(llvm::Type *T) { IRType = T; }

  bool isArg() { return IsArg; }

  llvm::StringRef getNamedArg() { return NamedArg; }
  void setNamedArg(llvm::StringRef Name) { NamedArg = Name; }

  AST_NODE(VarDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_VARDECL_H
