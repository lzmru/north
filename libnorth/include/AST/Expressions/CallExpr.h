
#ifndef LIBNORTH_AST_EXPRESSIONS_CALLEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_CALLEXPR_H

namespace north::ast {

class CallExpr : public Node {
public:
  struct Argument {
    Node *Arg;
    llvm::StringRef ArgName;
  };

private:
  std::vector<Argument *> Args;
  QualifiedIdentifierExpr *Ident;

  FunctionDecl *CallableFn = nullptr;
  llvm::Value *IRValue = nullptr;

public:
  explicit CallExpr(QualifiedIdentifierExpr *Identifier)
      : Node(Identifier->getPosition(), AST_CallExpr), Ident(Identifier)
  { assert(Identifier); }

  bool hasArgs() { return !Args.empty(); }
  size_t numberOfArgs() { return Args.size(); }
  llvm::ArrayRef<Argument *> getArgumentList() const { return Args; }
  Argument *getArg(size_t I) const { assert(I < Args.size()); return Args[I]; }

  void addArgument(Node *Arg, llvm::StringRef ArgName = "") {
    assert(Arg);
    Args.push_back(new Argument {Arg, ArgName});
  }
  
  void insertArgument(Node *Arg, llvm::StringRef ArgName = "") {
    assert(Arg);
    Args.insert(Args.begin(), new Argument {Arg, ArgName});
  }

  void setIdentifier(QualifiedIdentifierExpr *NewIdent) { assert(NewIdent); Ident = NewIdent; }
  QualifiedIdentifierExpr *getIdentifier() { return Ident; }
  llvm::StringRef getIdentifier(size_t I) { assert(I > Ident->getSize()); return Ident->getPart(I); }

  FunctionDecl *getCallableFn() { assert(CallableFn); return CallableFn; }
  void setCallableFn(FunctionDecl *, type::Module *);
  
  llvm::Value *getIR() { return IRValue; }
  void setIR(llvm::Value *IR) { assert(IR); IRValue = IR; }

  AST_NODE(CallExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_CALLEXPR_H
