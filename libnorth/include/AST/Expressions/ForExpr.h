
#ifndef LIBNORTH_AST_EXPRESSIONS_FOREXPR_H
#define LIBNORTH_AST_EXPRESSIONS_FOREXPR_H

namespace north::ast {

class ForExpr : public Node {
  Node *Iter;
  Node *Range;
  BlockStmt *Block;

public:
  explicit ForExpr(const TokenInfo &TkInfo)
      : Node(TkInfo.Pos, AST_ForExpr),
        Iter(nullptr), Range(nullptr), Block(nullptr) {}

  Node *getIter() { return Iter; }
  void setIter(LiteralExpr *NewIter) { Iter = NewIter; }

  Node *getRange() { return Range; }
  void setRange(Node *NewRange) { Range = NewRange; }

  BlockStmt *getBlock() { return Block; }
  void setBlock(BlockStmt *NewExpr) { Block = NewExpr; }

  AST_NODE(ForExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_FOREXPR_H
