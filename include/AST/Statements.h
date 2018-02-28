#ifndef NORTH_AST_STATEMENTS_H
#define NORTH_AST_STATEMENTS_H

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

  AST_NODE(BlockStmt)
};

class ReturnStmt : public Node {
  NodePtr Expr;

public:
  explicit ReturnStmt(const Position &Pos, Node *Expr = nullptr)
      : Node(Pos, AST_ReturnStmt), Expr(Expr) {}

  void setReturnExpr(Node *NewExpr) { Expr.reset(NewExpr); }
  Node *getReturnExpr() { return Expr.get(); }

  AST_NODE(ReturnStmt)
};

} // namespace north::ast

#endif // NORTH_AST_STATEMENTS_H
