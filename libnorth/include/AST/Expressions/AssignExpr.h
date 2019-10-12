
#ifndef LIBNORTH_AST_EXPRESSIONS_ASSIGNEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_ASSIGNEXPR_H

namespace north::ast {

class AssignExpr : public Node {
  Node *LHS;
  Token Operator;
  Node *RHS;

public:
  explicit AssignExpr(const TokenInfo &TkInfo, Node *LHS, Token Op, Node *RHS)
      : Node(TkInfo.Pos, AST_AssignExpr), LHS(LHS), Operator(Op), RHS(RHS) {}

  Node *getLHS() { return LHS; }
  void setLHS(Node *NewLHS) { LHS = NewLHS; }

  Node *getRHS() { return RHS; }
  void setRHS(Node *NewRHS) { RHS = NewRHS; }

  Token getOperator() { return Operator; }
  void setOperator(Token NewOp) { Operator = NewOp; }

  AST_NODE(AssignExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_ASSIGNEXPR_H
