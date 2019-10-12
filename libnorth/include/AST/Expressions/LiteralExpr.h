#ifndef LIBNORTH_AST_EXPRESSIONS_LITERALEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_LITERALEXPR_H

namespace north::ast {

class LiteralExpr : public Node {
  TokenInfo TkInfo;

public:
  explicit LiteralExpr(const TokenInfo &Info)
      : Node(Info.Pos, AST_LiteralExpr), TkInfo(Info) {}

  TokenInfo getTokenInfo() const { return TkInfo; }
  void setTokenInfo(TokenInfo NewType) { TkInfo = NewType; }

  AST_NODE(LiteralExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_LITERALEXPR_H
