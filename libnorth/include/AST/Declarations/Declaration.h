#ifndef LIBNORTH_AST_DECLARATIONS_DECLARATION_H
#define LIBNORTH_AST_DECLARATIONS_DECLARATION_H

namespace north::ast {

class Declaration : public Node {
  llvm::StringRef Identifier;

public:
  explicit Declaration(const Position &Pos, NodeKind Kind, llvm::StringRef Identifier)
      : Node(Pos, Kind), Identifier(Identifier) {}

  llvm::StringRef getIdentifier() { return Identifier; }
  void setIdentifier(llvm::StringRef NewIdent) { Identifier = NewIdent; }
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_DECLARATION_H
