
#ifndef LIBNORTH_AST_H
#define LIBNORTH_AST_H

#include "Grammar/Token.h"
#include "Visitor.h"

namespace llvm {
class Value;
}

namespace north::ast {

enum NodeKind {
  AST_TypeDef,

  AST_AliasDecl,
  AST_StructDecl,
  AST_UnionDecl,
  AST_EnumDecl,
  AST_TupleDecl,
  AST_RangeDecl,

  AST_InterfaceDecl,
  AST_FunctionDecl,
  AST_VarDecl,

  AST_BinaryExpr,
  AST_UnaryExpr,
  AST_LiteralExpr,
  AST_RangeExpr,
  AST_CallExpr,
  AST_ArrayIndexExpr,
  AST_QualifiedIdentifierExpr,
  AST_IfExpr,
  AST_ForExpr,
  AST_WhileExpr,
  AST_AssignExpr,
  AST_StructInitExpr,
  AST_ArrayExpr,

  AST_OpenStmt,
  AST_BlockStmt,
  AST_ReturnStmt,
};

class Node : public llvm::ilist_node<Node> {
  Position Pos;
  NodeKind Kind;

public:
  explicit Node(const Position &Pos, NodeKind Kind) : Pos(Pos), Kind(Kind) {}
  virtual ~Node() = default;

  const Position &getPosition() const { return Pos; }
  void setPosition(const Position &NewPos) { Pos = NewPos; }

  NodeKind getKind() const { return Kind; };

  virtual llvm::Value *accept(Visitor &) = 0;
};

using NodePtr = std::unique_ptr<Node>;

#define AST_NODE(KIND)                                                         \
  llvm::Value *accept(Visitor &V) override { return V.visit(*this); }          \
  static bool classof(const Node *Node) {                                      \
    return Node->getKind() == AST_##KIND;                                      \
  }

} // namespace north::ast

#include "Declarations.h"
#include "Expressions.h"
#include "Statements.h"

#endif // LIBNORTH_AST_H
