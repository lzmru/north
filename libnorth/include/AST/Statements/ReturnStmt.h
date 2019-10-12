#ifndef LIBNORTH_AST_STATEMENTS_RETURNSTMT_H
#define LIBNORTH_AST_STATEMENTS_RETURNSTMT_H

namespace north::ast {

class ReturnStmt : public Node {
  Node *Expr;

public:
  explicit ReturnStmt(const Position &Pos, Node *Expr = nullptr)
      : Node(Pos, AST_ReturnStmt), Expr(Expr) {}

  void setReturnExpr(Node *NewExpr) { Expr = NewExpr; }
  Node *getReturnExpr() { return Expr; }

  AST_NODE(ReturnStmt)
};

} // namespace north::ast

#endif // LIBNORTH_AST_STATEMENTS_RETURNSTMT_H
