
#ifndef NORTH_AST_VISITOR_H
#define NORTH_AST_VISITOR_H

#include <llvm/ADT/ilist.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>

namespace north::ast {

class Node;

class FunctionDecl;
class InterfaceDecl;
class VarDecl;
class AliasDecl;
class StructDecl;
class EnumDecl;
class UnionDecl;
class TupleDecl;
class RangeDecl;
class ArrayDecl;
class TypeDef;

class UnaryExpr;
class BinaryExpr;
class LiteralExpr;
class RangeExpr;
class CallExpr;
class IfExpr;
class ForExpr;
class WhileExpr;
class AssignExpr;
class StructInitExpr;
class ArrayExpr;

class OpenStmt;
class BlockStmt;
class ReturnStmt;

class Visitor {
public:
  virtual llvm::Value *visit(FunctionDecl &) { return nullptr; }
  virtual llvm::Value *visit(InterfaceDecl &) { return nullptr; }
  virtual llvm::Value *visit(VarDecl &) { return nullptr; }
  virtual llvm::Value *visit(AliasDecl &) { return nullptr; }
  virtual llvm::Value *visit(StructDecl &) { return nullptr; }
  virtual llvm::Value *visit(EnumDecl &) { return nullptr; }
  virtual llvm::Value *visit(UnionDecl &) { return nullptr; }
  virtual llvm::Value *visit(TupleDecl &) { return nullptr; }
  virtual llvm::Value *visit(RangeDecl &) { return nullptr; }
  virtual llvm::Value *visit(ArrayDecl &) { return nullptr; }
  virtual llvm::Value *visit(TypeDef &) { return nullptr; }

  virtual llvm::Value *visit(UnaryExpr &) { return nullptr; }
  virtual llvm::Value *visit(BinaryExpr &) { return nullptr; }
  virtual llvm::Value *visit(LiteralExpr &) { return nullptr; }
  virtual llvm::Value *visit(RangeExpr &) { return nullptr; }
  virtual llvm::Value *visit(CallExpr &) { return nullptr; }
  virtual llvm::Value *visit(IfExpr &) { return nullptr; }
  virtual llvm::Value *visit(ForExpr &) { return nullptr; }
  virtual llvm::Value *visit(WhileExpr &) { return nullptr; }
  virtual llvm::Value *visit(AssignExpr &) { return nullptr; }
  virtual llvm::Value *visit(StructInitExpr &) { return nullptr; }
  virtual llvm::Value *visit(ArrayExpr &) { return nullptr; }

  virtual llvm::Value *visit(OpenStmt &) { return nullptr; }
  virtual llvm::Value *visit(BlockStmt &) { return nullptr; }
  virtual llvm::Value *visit(ReturnStmt &) { return nullptr; }
};

#define AST_WALKER_METHODS                                                     \
  llvm::Value *visit(ast::FunctionDecl &) override;                            \
  llvm::Value *visit(ast::InterfaceDecl &) override;                           \
  llvm::Value *visit(ast::VarDecl &) override;                                 \
  llvm::Value *visit(ast::AliasDecl &) override;                               \
  llvm::Value *visit(ast::StructDecl &) override;                              \
  llvm::Value *visit(ast::EnumDecl &) override;                                \
  llvm::Value *visit(ast::UnionDecl &) override;                               \
  llvm::Value *visit(ast::TupleDecl &) override;                               \
  llvm::Value *visit(ast::RangeDecl &) override;                               \
  llvm::Value *visit(ast::TypeDef &) override;                                 \
  llvm::Value *visit(ast::UnaryExpr &) override;                               \
  llvm::Value *visit(ast::BinaryExpr &) override;                              \
  llvm::Value *visit(ast::LiteralExpr &) override;                             \
  llvm::Value *visit(ast::RangeExpr &) override;                               \
  llvm::Value *visit(ast::CallExpr &) override;                                \
  llvm::Value *visit(ast::IfExpr &) override;                                  \
  llvm::Value *visit(ast::ForExpr &) override;                                 \
  llvm::Value *visit(ast::WhileExpr &) override;                               \
  llvm::Value *visit(ast::AssignExpr &) override;                              \
  llvm::Value *visit(ast::OpenStmt &) override;                                \
  llvm::Value *visit(ast::BlockStmt &) override;                               \
  llvm::Value *visit(ast::ReturnStmt &) override;                              \
  llvm::Value *visit(ast::StructInitExpr &) override;                          \
  llvm::Value *visit(ast::ArrayExpr &) override;

} // namespace north::ast

#endif // NORTH_AST_VISITOR_H
