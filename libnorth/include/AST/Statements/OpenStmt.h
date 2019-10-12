#ifndef LIBNORTH_AST_STATEMENTS_OPENSTMT_H
#define LIBNORTH_AST_STATEMENTS_OPENSTMT_H

namespace north::ast {

class OpenStmt : public Node {
  llvm::StringRef ModuleName;

public:
  explicit OpenStmt(const Position &Pos, llvm::StringRef ModuleName)
      : Node(Pos, AST_OpenStmt), ModuleName(ModuleName) {}

  llvm::StringRef getModuleName() { return ModuleName; }
  void setModuleName(llvm::StringRef NewName) { ModuleName = NewName; }

  AST_NODE(OpenStmt)
};

} // namespace north::ast

#endif // LIBNORTH_AST_STATEMENTS_OPENSTMT_H
