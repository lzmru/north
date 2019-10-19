#ifndef LIBNORTH_AST_DECLARATIONS_UNIONDECL_H
#define LIBNORTH_AST_DECLARATIONS_UNIONDECL_H

namespace north::ast {

class UnionDecl final : public GenericDecl {
  std::vector<GenericDecl *> Fields;

public:
  explicit UnionDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_UnionDecl, "") {}

  const std::vector<GenericDecl *> &getFieldList() const { return Fields; }
  void addField(GenericDecl *Field) { Fields.push_back(Field); }

  AST_NODE(UnionDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_UNIONDECL_H
