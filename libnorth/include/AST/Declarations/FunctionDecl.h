#ifndef LIBNORTH_AST_DECLARATIONS_FUNCTIONDECL_H
#define LIBNORTH_AST_DECLARATIONS_FUNCTIONDECL_H

namespace north::ast {

class FunctionDecl : public GenericDecl {
  std::vector<VarDecl *> Arguments;
  BlockStmt *Block;
  llvm::Function *IRValue;
  bool IsVarArg;
  type::GenericFunction *GenericInstantiator = nullptr;
  type::Type *Type;

public:
  FunctionDecl(const TokenInfo &TkInfo, BlockStmt *Block = nullptr, bool VarArg = false)
      : GenericDecl(TkInfo.Pos, AST_FunctionDecl, TkInfo.toString()),
        Block(Block), IsVarArg(VarArg), Type(nullptr) {}

  FunctionDecl(const FunctionDecl &Fn) = default;

  bool hasArgs() const { return !Arguments.empty(); }
  llvm::ArrayRef<VarDecl *> getArgumentList() { return Arguments; }
  void addArgument(VarDecl *Argument) { Arguments.push_back(Argument); }
  VarDecl *getArg(uint8_t N) { return Arguments[N]; }

  BlockStmt *getBlockStmt() { return Block; }
  void setBlockStmt(BlockStmt *NewBlock) { Block = NewBlock; }

  void setIRValue(llvm::Function *Fn) { IRValue = Fn; }
  llvm::Function *getIRValue() { return IRValue; }

  void setVarArg(bool V) { IsVarArg = V; }
  bool isVarArg() { return IsVarArg; }

  void setGenericInstantiator(type::GenericFunction *New) { GenericInstantiator = New; }
  type::GenericFunction *getGenericInstantiator() { return GenericInstantiator; }

  type::Type *getType() { return Type; }
  void setType(type::Type *T) { Type = T; }
  void setType(GenericDecl *T) { Type = new type::Type(T); }

  AST_NODE(FunctionDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_FUNCTIONDECL_H
