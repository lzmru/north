
#ifndef LIBNORTH_AST_EXPRESSIONS_QUALIFIEDIDENTIFIEREXPR_H
#define LIBNORTH_AST_EXPRESSIONS_QUALIFIEDIDENTIFIEREXPR_H

namespace north::ast {

class QualifiedIdentifierExpr : public Node {
  std::vector<TokenInfo> Ident;

public:
  explicit QualifiedIdentifierExpr(const TokenInfo &TkInfo)
      : Node(TkInfo.Pos, AST_QualifiedIdentifierExpr) {
    Ident.push_back(TkInfo);
  }

  llvm::ArrayRef<TokenInfo> getIdentifier() { return Ident; }
  llvm::StringRef getPart(uint8_t I) { return Ident[I].toString(); }
  void removeFirst() { Ident.erase(Ident.begin()); }
  unsigned getSize() { return Ident.size(); }
  void AddPart(const TokenInfo &TkInfo) { Ident.push_back(TkInfo); }

  AST_NODE(QualifiedIdentifierExpr)
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_QUALIFIEDIDENTIFIEREXPR_H
