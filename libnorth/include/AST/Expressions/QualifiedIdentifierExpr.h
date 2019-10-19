
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

  llvm::ArrayRef<TokenInfo> getIdentifier() const { return Ident; }
  llvm::StringRef getPart(uint8_t I) const { return Ident[I].toString(); }
  void removeFirst() { Ident.erase(Ident.begin()); }
  unsigned getSize() const { return Ident.size(); }
  void AddPart(const TokenInfo &TkInfo) { Ident.push_back(TkInfo); }

  AST_NODE(QualifiedIdentifierExpr)
  
  bool operator==(const QualifiedIdentifierExpr &) const;
};

} // namespace north::ast

#endif // LIBNORTH_AST_EXPRESSIONS_QUALIFIEDIDENTIFIEREXPR_H
