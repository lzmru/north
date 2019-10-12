#ifndef LIBNORTH_AST_STATEMENTS_BLOCKSTMT_H
#define LIBNORTH_AST_STATEMENTS_BLOCKSTMT_H

namespace north::ast {

class BlockStmt : public Node {
  FunctionDecl *Owner;
  BlockStmt *ParentBlock;
  llvm::simple_ilist<ast::Node> *Body;

public:
  explicit BlockStmt(const Position &Pos, BlockStmt *Parent = nullptr)
      : Node(Pos, AST_BlockStmt), Owner(nullptr), ParentBlock(Parent),
        Body(new llvm::simple_ilist<ast::Node>()) {}

  void addNode(Node *NewNode) { Body->push_back(*NewNode); }
  llvm::simple_ilist<ast::Node> *getBody() { return Body; }

  void setOwner(FunctionDecl *Fn) { Owner = Fn; }
  FunctionDecl *getOwner() const { return Owner; }
  
  BlockStmt *getParent() { return ParentBlock; }

  AST_NODE(BlockStmt)
};

} // namespace north::ast

#endif // LIBNORTH_AST_STATEMENTS_BLOCKSTMT_H
