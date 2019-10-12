#ifndef LIBNORTH_AST_DECLARATIONS_RANGEDECL_H
#define LIBNORTH_AST_DECLARATIONS_RANGEDECL_H

namespace north::ast {

class RangeDecl : public GenericDecl {
  std::vector<RangeExpr *> Ranges;

public:
  explicit RangeDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_RangeDecl, "") {}

  const std::vector<RangeExpr *> &getRangeList() const { return Ranges; }
  void addRange(RangeExpr *Range) { Ranges.push_back(Range); }

  AST_NODE(RangeDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_RANGEDECL_H
