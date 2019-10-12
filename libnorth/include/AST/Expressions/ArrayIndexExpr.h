
#ifndef LIBNORTH_AST_EXPRESSIONS_ARRAYINDEXEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_ARRAYINDEXEXPR_H

namespace north::ast {

class ArrayIndexExpr : public Node {
  Node *Ident;
  Node *IdxExpr;

public:
  explicit ArrayIndexExpr(Node *Identifier)
      : Node(Identifier->getPosition(), AST_ArrayIndexExpr), Ident(Identifier) {
  }

  void setIdxExpr(Node *Idx) { IdxExpr = Idx; }
  Node *getIdxExpr() { return IdxExpr; }

  Node *getIdentifier() { return Ident; }

  AST_NODE(ArrayIndexExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_ARRAYINDEXEXPR_H
