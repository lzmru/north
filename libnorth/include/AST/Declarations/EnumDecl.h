#ifndef LIBNORTH_AST_DECLARATIONS_ENUMDECL_H
#define LIBNORTH_AST_DECLARATIONS_ENUMDECL_H

#include <llvm/ADT/StringMap.h>

namespace north::ast {

class EnumDecl : public GenericDecl {
  std::vector<LiteralExpr> Members;
  llvm::StringMap<llvm::Value *> IRValues;

public:
  explicit EnumDecl(const TokenInfo &Info)
      : GenericDecl(Info.Pos, AST_EnumDecl, "") {
    Members.emplace_back(Info);
  }

  llvm::ArrayRef<LiteralExpr> getMemberList() const { return Members; }
  void addMember(const TokenInfo &Info) { Members.emplace_back(Info); }

  llvm::Value *getValue(llvm::StringRef K) const {
    return IRValues.find(K)->second;
  }
  void addValue(llvm::StringRef K, llvm::Value *V) {
    // TODO: error reporting
    IRValues.try_emplace(K, V);
  }

  AST_NODE(EnumDecl)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_ENUMDECL_H
