
#ifndef LIBNORTH_AST_EXPRESSIONS_IFEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_IFEXPR_H

namespace north::ast {

class IfExpr : public Node {
  Node *Expr;
  BlockStmt *Block;
  IfExpr *ElseBranch;

public:
  explicit IfExpr(const TokenInfo &TkInfo, Node *Expr = nullptr,
                  BlockStmt *Block = nullptr)
      : Node(TkInfo.Pos, AST_IfExpr), Expr(Expr), Block(Block) {}

  Node *getExpr() { return Expr; }
  void setExpr(Node *NewExpr) { Expr = NewExpr; }

  BlockStmt *getBlock() { return Block; }
  void setBlock(BlockStmt *NewExpr) { Block = NewExpr; }

  IfExpr *getElseBranch() { return ElseBranch; }
  void setElseBranch(IfExpr *NewExpr) { ElseBranch = NewExpr; }
  bool hasElse() { return ElseBranch; }

  AST_NODE(IfExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_IFEXPR_H
