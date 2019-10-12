
#ifndef LIBNORTH_AST_H
#define LIBNORTH_AST_H

#include "Grammar/Token.h"
#include "Visitor.h"
#include "Type/Type.h"

#include <llvm/ADT/ArrayRef.h>

namespace llvm {
class Function;
class Type;
class Value;
class StructType;
} // namespace llvm

namespace north::type {
class GenericFunction;
} // namespace north::type

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

#define AST_NODE(KIND)                                                         \
  llvm::Value *accept(Visitor &V) override { return V.visit(*this); }          \
  static bool classof(const Node *Node) {                                      \
    return Node->getKind() == AST_##KIND;                                      \
  }

} // namespace north::ast

#include "Declarations/Declaration.h"
#include "Declarations/GenericDecl.h"
#include "Declarations/VarDecl.h"
#include "Declarations/InterfaceDecl.h"
#include "Declarations/AliasDecl.h"
#include "Declarations/StructDecl.h"
#include "Declarations/EnumDecl.h"
#include "Declarations/FunctionDecl.h"
#include "Declarations/UnionDecl.h"
#include "Declarations/TupleDecl.h"
#include "Declarations/RangeDecl.h"
#include "Declarations/TypeDef.h"

#include "Expressions/UnaryExpr.h"
#include "Expressions/BinaryExpr.h"
#include "Expressions/LiteralExpr.h"
#include "Expressions/RangeExpr.h"
#include "Expressions/ArrayIndexExpr.h"
#include "Expressions/QualifiedIdentifierExpr.h"
#include "Expressions/CallExpr.h"
#include "Expressions/IfExpr.h"
#include "Expressions/ForExpr.h"
#include "Expressions/WhileExpr.h"
#include "Expressions/AssignExpr.h"
#include "Expressions/StructInitExpr.h"
#include "Expressions/ArrayExpr.h"

#include "Statements/OpenStmt.h"
#include "Statements/BlockStmt.h"
#include "Statements/ReturnStmt.h"

#endif // LIBNORTH_AST_H
