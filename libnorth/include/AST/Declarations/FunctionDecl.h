#ifndef LIBNORTH_AST_DECLARATIONS_FUNCTIONDECL_H
#define LIBNORTH_AST_DECLARATIONS_FUNCTIONDECL_H

namespace north::ast {

class FunctionDecl : public GenericDecl {
  std::vector<VarDecl *> Arguments;
  GenericDecl *Type = nullptr;
  BlockStmt *Block;
  
  bool IsVarArg;
  
  llvm::Function *IR = nullptr;
  llvm::Type *TypeIR = nullptr;

public:
  FunctionDecl(const TokenInfo &TkInfo, BlockStmt *Block = nullptr, bool VarArg = false)
      : GenericDecl(TkInfo.Pos, AST_FunctionDecl, TkInfo.toString()),
        Block(Block), IsVarArg(VarArg) {}

  FunctionDecl(const FunctionDecl &Fn) = default;

  bool hasArgs() const { return !Arguments.empty(); }
  llvm::ArrayRef<VarDecl *> getArgumentList() { return Arguments; }
  size_t countOfArgs() { return Arguments.size(); }
  
  VarDecl *getArg(size_t N) {
    if (N >= Arguments.size())
      return nullptr;
    return Arguments[N];
  }
  
  void addArgument(VarDecl *Argument) {
    assert(Argument);
    Arguments.push_back(Argument);
  }
  
  GenericDecl *getTypeDecl() { return Type; }
  void setTypeDecl(GenericDecl *T) { Type = T; }
  
  llvm::Type *getTypeIR() { return TypeIR; }
  void setTypeIR(llvm::Type *T) { TypeIR = T; }

  BlockStmt *getBlockStmt() { return Block; }
  void setBlockStmt(BlockStmt *NewBlock) { assert(NewBlock); Block = NewBlock; }

  llvm::Function *getIR() {
    assert(IR && "createIR() must be called before");
    return IR;
  }
  
  llvm::Function *maybeGetIR() { return IR; }

  void setVarArg(bool V) { IsVarArg = V; }
  bool isVarArg() { return IsVarArg; }

  AST_NODE(FunctionDecl)
  
  void createIR(type::Module *);
};

class GenericFunctionDecl : public FunctionDecl {
public:
  struct InstantiatedFn {
    FunctionDecl *Fn;
    llvm::SmallVector<GenericDecl::Generic, 2> Types;
  };
  
private:
  llvm::SmallVector<CallExpr *, 4> Calls;
  llvm::SmallVector<InstantiatedFn, 4> InstantiatedFunctions;
  
  FunctionDecl *isInstantiatedAlready(llvm::ArrayRef<GenericDecl::Generic>) const;
  
public:
  GenericFunctionDecl(const TokenInfo &TkInfo, BlockStmt *Block = nullptr, bool VarArg = false)
    : FunctionDecl(TkInfo, Block, VarArg) {}
  
  void addInstantiatedFunction(const InstantiatedFn &Fn) { InstantiatedFunctions.push_back(Fn); }
  llvm::ArrayRef<InstantiatedFn> getInstantiatedFunctions() const { return InstantiatedFunctions; }
  
  AST_NODE(GenericFunctionDecl)
  
  void addCallExpr(CallExpr *);
  llvm::ArrayRef<CallExpr *> getCalls() { return Calls; }
  FunctionDecl *instantiate(ast::CallExpr *, type::Module *);
};
  
} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_FUNCTIONDECL_H
