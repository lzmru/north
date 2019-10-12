
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
  llvm::Value *IRValue;

public:
  explicit CallExpr(QualifiedIdentifierExpr *Identifier)
      : Node(Identifier->getPosition(), AST_CallExpr), Ident(Identifier) {}

  bool hasArgs() { return !Args.empty(); }
  size_t numberOfArgs() { return Args.size(); }
  llvm::ArrayRef<Argument *> getArgumentList() const { return Args; }
  Argument *getArg(size_t I) const { return Args[I]; }

  void addArgument(Node *Arg, llvm::StringRef ArgName = "") {
    Args.push_back(new Argument {Arg, ArgName});
  }
  void insertArgument(Node *Arg, llvm::StringRef ArgName = "") {
    Args.insert(Args.begin(), new Argument {Arg, ArgName});
  }

  void setIdentifier(QualifiedIdentifierExpr *NewIdent) { Ident = NewIdent; }
  QualifiedIdentifierExpr *getIdentifier() { return Ident; }
  llvm::StringRef getIdentifier(size_t I) { return Ident->getPart(I); }

  llvm::Value *getIR() { return IRValue; }
  void setIR(llvm::Value *Val) { IRValue = Val; }

  AST_NODE(CallExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_CALLEXPR_H
