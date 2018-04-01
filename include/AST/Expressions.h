
#ifndef NORTH_AST_EXPRESSIONS_H
#define NORTH_AST_EXPRESSIONS_H

#include <llvm/ADT/ArrayRef.h>
#include <vector>

namespace north::ast {

class UnaryExpr : public Node {
  Token Operator;
  NodePtr Operand;

public:
  explicit UnaryExpr(const TokenInfo &TkInfo, Token Operator, Node *Operand)
      : Node(TkInfo.Pos, AST_UnaryExpr), Operator(Operator), Operand(Operand) {}

  Token getOperator() { return Operator; }
  void setOperator(Token NewOp) { Operator = NewOp; }

  Node *getOperand() const { return Operand.get(); }
  void setOperand(Node *NewOperand) { Operand.reset(NewOperand); }

  AST_NODE(UnaryExpr)
};

class BinaryExpr : public Node {
  NodePtr LHS;
  Token Operator;
  NodePtr RHS;

public:
  explicit BinaryExpr(const TokenInfo &TkInfo, Node *LHS, Token Operator,
                      Node *RHS)
      : Node(TkInfo.Pos, AST_BinaryExpr), LHS(LHS), Operator(Operator),
        RHS(RHS) {}

  Node *getLHS() const { return LHS.get(); }
  void setLHS(Node *NewLHS) { LHS.reset(NewLHS); }

  Token getOperator() { return Operator; }
  void setOperator(Token NewOp) { Operator = NewOp; }

  Node *getRHS() const { return RHS.get(); }
  void setRHS(Node *NewRHS) { RHS.reset(NewRHS); }

  AST_NODE(BinaryExpr)
};

class LiteralExpr : public Node {
  TokenInfo TkInfo;

public:
  explicit LiteralExpr(const TokenInfo &Info)
      : Node(Info.Pos, AST_LiteralExpr), TkInfo(Info) {}

  TokenInfo getTokenInfo() const { return TkInfo; }
  void setTokenInfo(TokenInfo NewType) { TkInfo = NewType; }

  AST_NODE(LiteralExpr)
};

class RangeExpr : public Node {
  std::unique_ptr<LiteralExpr> Begin, End;

public:
  explicit RangeExpr(const TokenInfo &TkInfo)
      : Node(TkInfo.Pos, AST_RangeExpr), Begin(new LiteralExpr(TkInfo)),
        End(nullptr) {}

  LiteralExpr *getBeginValue() const { return Begin.get(); }
  void setBeginValue(LiteralExpr *BeginValue) { Begin.reset(BeginValue); }

  LiteralExpr *getEndValue() const { return End.get(); }
  void setEndValue(LiteralExpr *EndValue) { End.reset(EndValue); }

  AST_NODE(RangeExpr)
};

class CallExpr : public Node {
  std::vector<Node *> Args;
  llvm::StringRef Ident;

public:
  explicit CallExpr(Node *Identifier)
      : Node(Identifier->getPosition(), AST_CallExpr),
        Ident(Identifier->getPosition().Offset,
              Identifier->getPosition().Length) {}

  bool hasArgs() { return !Args.empty(); }
  size_t numberOfArgs() { return Args.size(); }
  llvm::ArrayRef<Node *> getArgumentList() const { return Args; }
  void addArgument(Node *Argument) { Args.push_back(Argument); }

  void setIdentifier(llvm::StringRef NewIdent) { Ident = NewIdent; }
  llvm::StringRef getIdentifier() { return Ident; }

  AST_NODE(CallExpr)
};

class ArrayIndexExpr : public Node {
  Node *Ident;
  Node *IdxExpr;

public:
  explicit ArrayIndexExpr(Node *Identifier)
      : Node(Identifier->getPosition(), AST_ArrayIndexExpr), Ident(Identifier) {
  }

  void setIdxExpr(Node *Idx) { IdxExpr = Idx; }
  Node *getIdxExpr() { return IdxExpr; }

  Node *getIdentifier() { return Ident; }

  AST_NODE(ArrayIndexExpr)
};

class QualifiedIdentifierExpr : public Node {
  std::vector<TokenInfo> Ident;

public:
  explicit QualifiedIdentifierExpr(const TokenInfo &TkInfo)
      : Node(TkInfo.Pos, AST_QualifiedIdentifierExpr) {
    Ident.push_back(TkInfo);
  }

  llvm::ArrayRef<TokenInfo> getIdentifier() { return Ident; }
  llvm::StringRef getPart(uint8_t I) { return Ident[I].toString(); }
  unsigned getSize() { return Ident.size(); }
  void AddPart(const TokenInfo &TkInfo) { Ident.push_back(TkInfo); }

  AST_NODE(QualifiedIdentifierExpr)
};

class IfExpr : public Node {
  NodePtr Expr;
  std::unique_ptr<BlockStmt> Block;
  std::unique_ptr<IfExpr> ElseBranch;

public:
  explicit IfExpr(const TokenInfo &TkInfo, Node *Expr = nullptr,
                  BlockStmt *Block = nullptr)
      : Node(TkInfo.Pos, AST_IfExpr), Expr(Expr), Block(Block) {}

  Node *getExpr() { return Expr.get(); }
  void setExpr(Node *NewExpr) { Expr.reset(NewExpr); }

  BlockStmt *getBlock() { return Block.get(); }
  void setBlock(BlockStmt *NewExpr) { Block.reset(NewExpr); }

  IfExpr *getElseBranch() { return ElseBranch.get(); }
  void setElseBranch(IfExpr *NewExpr) { ElseBranch.reset(NewExpr); }
  bool hasElse() { return ElseBranch.get(); }

  AST_NODE(IfExpr)
};

class ForExpr : public Node {
  std::unique_ptr<Node> Iter;
  NodePtr Range;
  std::unique_ptr<BlockStmt> Block;

public:
  explicit ForExpr(const TokenInfo &TkInfo)
      : Node(TkInfo.Pos, AST_ForExpr), Iter(nullptr), Range(nullptr),
        Block(nullptr) {}

  Node *getIter() { return Iter.get(); }
  void setIter(LiteralExpr *NewIter) { Iter.reset(NewIter); }

  Node *getRange() { return Range.get(); }
  void setRange(Node *NewRange) { Range.reset(NewRange); }

  BlockStmt *getBlock() { return Block.get(); }
  void setBlock(BlockStmt *NewExpr) { Block.reset(NewExpr); }

  AST_NODE(ForExpr)
};

class WhileExpr : public Node {
  NodePtr Expr;
  std::unique_ptr<BlockStmt> Block;

public:
  explicit WhileExpr(const TokenInfo &TkInfo, Node *Expr)
      : Node(TkInfo.Pos, AST_WhileExpr), Expr(Expr), Block(nullptr) {}

  Node *getExpr() { return Expr.get(); }
  void setExpr(Node *NewExpr) { Expr.reset(NewExpr); }

  BlockStmt *getBlock() { return Block.get(); }
  void setBlock(BlockStmt *NewExpr) { Block.reset(NewExpr); }

  AST_NODE(WhileExpr)
};

class AssignExpr : public Node {
  NodePtr LHS;
  Token Operator;
  NodePtr RHS;

public:
  explicit AssignExpr(const TokenInfo &TkInfo, Node *LHS, Token Op, Node *RHS)
      : Node(TkInfo.Pos, AST_AssignExpr), LHS(LHS), Operator(Op), RHS(RHS) {}

  Node *getLHS() { return LHS.get(); }
  void setLHS(Node *NewLHS) { LHS.reset(NewLHS); }

  Node *getRHS() { return RHS.get(); }
  void setRHS(Node *NewRHS) { RHS.reset(NewRHS); }

  Token getOperator() { return Operator; }
  void setOperator(Token NewOp) { Operator = NewOp; }

  AST_NODE(AssignExpr)
};

class StructInitExpr : public Node {
  Node *Identifier;
  std::vector<Node *> Values;
  StructDecl *Type;
  llvm::Constant *IRValue;

public:
  explicit StructInitExpr(Node *Ident)
      : Node(Ident->getPosition(), AST_StructInitExpr), Identifier(Ident),
        Type(nullptr), IRValue(nullptr) {}

  void addValue(Node *Val) { Values.push_back(Val); }
  llvm::ArrayRef<Node *> getValues() { return Values; }
  Node *getValue(size_t I) { return Values[I]; }
  Node *getIdentifier() { return Identifier; }

  void setType(StructDecl *T) { Type = T; }
  StructDecl *getType() { return Type; }

  void setIRValue(llvm::Constant *Val) { IRValue = Val; }
  llvm::Constant *getIRValue() { return IRValue; }

  AST_NODE(StructInitExpr)
};

class ArrayExpr : public Node {
  std::vector<Node *> Values;

public:
  explicit ArrayExpr(const TokenInfo &LBracket)
      : Node(LBracket.Pos, AST_ArrayExpr) {}

  void addValue(Node *Val) { Values.push_back(Val); }
  llvm::ArrayRef<Node *> getValues() { return Values; }
  Node *getValue(size_t Idx) { return Values[Idx]; }
  size_t getCap() { return Values.size(); }

  AST_NODE(ArrayExpr)
};

} // namespace north::ast

#endif // NORTH_AST_EXPRESSIONS_H
