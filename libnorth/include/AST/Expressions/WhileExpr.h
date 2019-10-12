
#ifndef LIBNORTH_AST_EXPRESSIONS_WHILEEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_WHILEEXPR_H

namespace north::ast {

class WhileExpr : public Node {
  Node *Expr;
  BlockStmt *Block;

public:
  explicit WhileExpr(const TokenInfo &TkInfo, Node *Expr)
      : Node(TkInfo.Pos, AST_WhileExpr), Expr(Expr), Block(nullptr) {}

  Node *getExpr() { return Expr; }
  void setExpr(Node *NewExpr) { Expr = NewExpr; }

  BlockStmt *getBlock() { return Block; }
  void setBlock(BlockStmt *NewExpr) { Block = NewExpr; }

  AST_NODE(WhileExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_WHILEEXPR_H
