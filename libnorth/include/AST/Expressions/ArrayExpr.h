
#ifndef LIBNORTH_AST_EXPRESSIONS_ARRAYEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_ARRAYEXPR_H

namespace north::ast {

class ArrayExpr : public Node {
  std::vector<Node *> Values;

public:
  explicit ArrayExpr(const TokenInfo &LBracket)
      : Node(LBracket.Pos, AST_ArrayExpr) {}

  void addValue(Node *Val) { Values.push_back(Val); }
  llvm::ArrayRef<Node *> getValues() { return Values; }
  Node *getValue(size_t Idx) { return Values[Idx]; }
  size_t getCap() { return Values.size(); }

  AST_NODE(ArrayExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_ARRAYEXPR_H
