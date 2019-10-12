
#ifndef LIBNORTH_AST_EXPRESSIONS_BINARYEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_BINARYEXPR_H

namespace north::ast {

class BinaryExpr : public Node {
  Node *LHS;
  Token Operator;
  Node *RHS;

public:
  explicit BinaryExpr(const TokenInfo &TkInfo, Node *LHS, Token Operator,
                      Node *RHS)
      : Node(TkInfo.Pos, AST_BinaryExpr), LHS(LHS), Operator(Operator),
        RHS(RHS) {}

  Node *getLHS() const { return LHS; }
  void setLHS(Node *NewLHS) { LHS = NewLHS; }

  Token getOperator() { return Operator; }
  void setOperator(Token NewOp) { Operator = NewOp; }

  Node *getRHS() const { return RHS; }
  void setRHS(Node *NewRHS) { RHS = NewRHS; }

  AST_NODE(BinaryExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_BINARYEXPR_H
