
#ifndef LIBNORTH_AST_EXPRESSIONS_STRUCTINITEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_STRUCTINITEXPR_H

namespace north::ast {

class StructInitExpr : public Node {
  Node *Identifier;
  std::vector<Node *> Values;
  StructDecl *Type;
  llvm::Constant *IRValue;

public:
  explicit StructInitExpr(Node *Ident)
      : Node(Ident->getPosition(), AST_StructInitExpr), Identifier(Ident),
        Type(nullptr), IRValue(nullptr) {}

  void addValue(Node *Val) { Values.push_back(Val); }
  llvm::ArrayRef<Node *> getValues() { return Values; }
  Node *getValue(size_t I) { return Values[I]; }
  Node *getIdentifier() { return Identifier; }

  void setType(StructDecl *T) { Type = T; }
  StructDecl *getType() { return Type; }

  void setIRValue(llvm::Constant *Val) { IRValue = Val; }
  llvm::Constant *getIRValue() { return IRValue; }

  AST_NODE(StructInitExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_STRUCTINITEXPR_H
