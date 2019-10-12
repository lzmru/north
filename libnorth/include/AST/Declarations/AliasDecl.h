#ifndef LIBNORTH_AST_DECLARATIONS_ALIASDECL_H
#define LIBNORTH_AST_DECLARATIONS_ALIASDECL_H

namespace north::ast {

class AliasDecl : public GenericDecl {
  llvm::StringRef Alias; // TODO: Twine for qualified names
  llvm::Type *IRValue;

public:
  explicit AliasDecl(const Position &Pos, llvm::StringRef Identifier,
                     llvm::StringRef Alias = "")
      : GenericDecl(Pos, AST_AliasDecl, Identifier), Alias(Alias) {}

  llvm::StringRef getAlias() { return Alias; }
  void setAlias(llvm::StringRef NewAlias) { Alias = NewAlias; }

  llvm::Type *getIR() { return IRValue; }
  void setIR(llvm::Type *NewIRValue) { IRValue = NewIRValue; }

  AST_NODE(AliasDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_ALIASDECL_H
