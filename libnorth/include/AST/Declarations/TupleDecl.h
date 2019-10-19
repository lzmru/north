#ifndef LIBNORTH_AST_DECLARATIONS_TUPLEDECL_H
#define LIBNORTH_AST_DECLARATIONS_TUPLEDECL_H

namespace north::ast {

class TupleDecl final : public GenericDecl {
  std::vector<VarDecl *> Members;

public:
  explicit TupleDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_TupleDecl, "") {}

  const std::vector<VarDecl *> &getMemberList() const { return Members; }
  void addMember(VarDecl *Type) { Members.push_back(Type); }

  AST_NODE(TupleDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_TUPLEDECL_H
