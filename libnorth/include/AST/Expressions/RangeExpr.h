
#ifndef LIBNORTH_AST_EXPRESSIONS_RANGEEXPR_H
#define LIBNORTH_AST_EXPRESSIONS_RANGEEXPR_H

namespace north::ast {

class RangeExpr : public Node {
  LiteralExpr *Begin, *End;

public:
  explicit RangeExpr(const TokenInfo &TkInfo)
      : Node(TkInfo.Pos, AST_RangeExpr), Begin(new LiteralExpr(TkInfo)),
        End(nullptr) {}

  LiteralExpr *getBeginValue() const { return Begin; }
  void setBeginValue(LiteralExpr *BeginValue) { Begin = BeginValue; }

  LiteralExpr *getEndValue() const { return End; }
  void setEndValue(LiteralExpr *EndValue) { End = EndValue; }

  AST_NODE(RangeExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_RANGEEXPR_H
