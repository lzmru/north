
#ifndef LIBNORTH_AST_EXPRESSIONS_UNARYEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_UNARYEXPR_H

namespace north::ast {

class UnaryExpr : public Node {
  Token Operator;
  Node *Operand;

public:
  explicit UnaryExpr(const TokenInfo &TkInfo, Token Operator, Node *Operand)
      : Node(TkInfo.Pos, AST_UnaryExpr), Operator(Operator), Operand(Operand) {}

  Token getOperator() { return Operator; }
  void setOperator(Token NewOp) { Operator = NewOp; }

  Node *getOperand() const { return Operand; }
  void setOperand(Node *NewOperand) { Operand = NewOperand; }

  AST_NODE(UnaryExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_UNARYEXPR_H
