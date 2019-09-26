#ifndef LIBNORTH_AST_DECLARATIONS_H
#define LIBNORTH_AST_DECLARATIONS_H

#include "AST.h"

#include <vector>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/DerivedTypes.h>

namespace llvm {
class Function;
} // namespace llvm

namespace north::ast {

class Declaration : public Node {
  llvm::StringRef Identifier;

public:
  explicit Declaration(const Position &Pos, NodeKind Kind,
                       llvm::StringRef Identifier)
      : Node(Pos, Kind), Identifier(Identifier) {}

  llvm::StringRef getIdentifier() { return Identifier; }
  void setIdentifier(llvm::StringRef NewIdent) { Identifier = NewIdent; }
};

class GenericDecl : public Declaration {
public:
  struct Generic {
    Position Pos;
    llvm::StringRef Name;

    explicit Generic(Position Pos, llvm::StringRef Name)
        : Pos(Pos), Name(Name) {}
  };
  using GenericList = std::vector<Generic>;

  enum Modifier { None = 0, Ptr = 1, Ref = 2, Out = 4, In = 8 };

private:
  uint8_t Modifiers;
  GenericList Generics;

public:
  explicit GenericDecl(const Position &Pos, NodeKind Kind,
                       llvm::StringRef Identifier, Modifier Mods = None)
      : Declaration(Pos, Kind, Identifier), Modifiers(Mods) {}

  bool hasGenerics() { return !Generics.empty(); }
  const GenericList &getGenericsList() const { return Generics; }
  void addGenericType(const Position &Pos, llvm::StringRef Identifier) {
    Generics.emplace_back(Pos, Identifier);
  }

  void setModifier(Modifier M) { Modifiers |= M; }
  bool hasModifier() { return Modifiers != 0; }
  bool isPtr() { return (Modifiers & Ptr) == Ptr; }
  bool isRef() { return (Modifiers & Ref) == Ref; }
  bool isOut() { return (Modifiers & Out) == Out; }
  bool isIn() { return (Modifiers & In) == In; }
};

class VarDecl : public Declaration {
  std::unique_ptr<GenericDecl> Type;
  std::unique_ptr<Node> Value;
  llvm::Value *IRValue;
  llvm::Type *IRType;
  bool IsArg;
  llvm::StringRef NamedArg;

public:
  explicit VarDecl(const TokenInfo &TkInfo, bool Arg = false)
      : Declaration(TkInfo.Pos, AST_VarDecl, TkInfo.toString()), Type(nullptr),
        Value(nullptr), IRValue(nullptr), IsArg(Arg) {}

  GenericDecl *getType() { return Type.get(); }
  void setType(GenericDecl *NewType) { Type.reset(NewType); }

  Node *getValue() { return Value.get(); }
  void setValue(Node *NewValue) { Value.reset(NewValue); }

  llvm::Value *getIRValue() { return IRValue; }
  void setIRValue(llvm::Value *Val) { IRValue = Val; }

  llvm::Type *getIRType() { return IRType; }
  void setIRType(llvm::Type *T) { IRType = T; }

  bool isArg() { return IsArg; }

  llvm::StringRef getNamedArg() { return NamedArg; }
  void setNamedArg(llvm::StringRef Name) { NamedArg = Name; }

  AST_NODE(VarDecl)
};

class InterfaceDecl : public GenericDecl {
  using RequiredList = std::vector<FunctionDecl *>;
  RequiredList Required;
  std::unique_ptr<InterfaceDecl> Parent;

public:
  explicit InterfaceDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_InterfaceDecl, TkInfo.toString()),
        Parent(nullptr) {}

  void addFunction(FunctionDecl *Function) { Required.push_back(Function); }
  const RequiredList &getDemands() const { return Required; }

  InterfaceDecl *getParent() { return Parent.get(); }
  void setParent(InterfaceDecl *NewParent) { Parent.reset(NewParent); }

  AST_NODE(InterfaceDecl)
};

class AliasDecl : public GenericDecl {
  llvm::StringRef Alias; // TODO: Twine for qualified names
  llvm::Type *IRValue;

public:
  explicit AliasDecl(const Position &Pos, llvm::StringRef Identifier,
                     llvm::StringRef Alias = "")
      : GenericDecl(Pos, AST_AliasDecl, Identifier), Alias(Alias) {}

  llvm::StringRef getAlias() { return Alias; }
  void setAlias(llvm::StringRef NewAlias) { Alias = NewAlias; }

  llvm::Type *getIR() { return IRValue; }
  void setIR(llvm::Type *NewIRValue) { IRValue = NewIRValue; }

  AST_NODE(AliasDecl)
};

class StructDecl : public GenericDecl {
  std::vector<VarDecl *> Fields;
  llvm::StructType *IRValue;

public:
  explicit StructDecl(const Position &Pos, bool IsPacked = false)
      : GenericDecl(Pos, AST_StructDecl, "") {}

  llvm::ArrayRef<VarDecl *> getFieldList() { return Fields; }
  VarDecl *getField(uint8_t I) { return Fields[I]; }
  void addField(VarDecl *Field) { Fields.push_back(Field); }

  llvm::StructType *getIR() { return IRValue; }
  void setIR(llvm::Type *NewIRValue) {
    IRValue = static_cast<llvm::StructType *>(NewIRValue);
  }

  AST_NODE(StructDecl)
};

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

class FunctionDecl : public GenericDecl {
  std::vector<VarDecl *> Arguments;
  std::unique_ptr<GenericDecl> Type;
  std::unique_ptr<BlockStmt> Block;
  llvm::Function *IRValue;
  bool IsVarArg;

public:
  explicit FunctionDecl(const TokenInfo &TkInfo, GenericDecl *Type = nullptr,
                        BlockStmt *Block = nullptr, bool VarArg = false)
      : GenericDecl(TkInfo.Pos, AST_FunctionDecl, TkInfo.toString()),
        Type(Type), Block(Block), IsVarArg(VarArg) {}

  bool hasArgs() const { return !Arguments.empty(); }
  llvm::ArrayRef<VarDecl *> getArgumentList() { return Arguments; }
  void addArgument(VarDecl *Argument) { Arguments.push_back(Argument); }
  VarDecl *getArg(uint8_t N) { return Arguments[N]; }

  BlockStmt *getBlockStmt() { return Block.get(); }
  void setBlockStmt(BlockStmt *NewBlock) { Block.reset(NewBlock); }

  GenericDecl *getTypeDecl() { return Type.get(); }
  void setTypeDecl(GenericDecl *NewType) { Type.reset(NewType); }

  void setIRValue(llvm::Function *Fn) { IRValue = Fn; }
  llvm::Function *getIRValue() { return IRValue; }

  void setVarArg(bool V) { IsVarArg = V; }
  bool isVarArg() { return IsVarArg; }

  AST_NODE(FunctionDecl)
};

class UnionDecl : public GenericDecl {
  std::vector<GenericDecl *> Fields;

public:
  explicit UnionDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_UnionDecl, "") {}

  const std::vector<GenericDecl *> &getFieldList() const { return Fields; }
  void addField(GenericDecl *Field) { Fields.push_back(Field); }

  AST_NODE(UnionDecl)
};

class TupleDecl : public GenericDecl {
  std::vector<VarDecl *> Members; // TODO: free memory in destructor or use
                                  // something like boost::ptr_vector

public:
  explicit TupleDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_TupleDecl, "") {}

  const std::vector<VarDecl *> &getMemberList() const { return Members; }
  void addMember(VarDecl *Type) { Members.push_back(Type); }

  AST_NODE(TupleDecl)
};

class RangeDecl : public GenericDecl {
  std::vector<RangeExpr *> Ranges;

public:
  explicit RangeDecl(const TokenInfo &TkInfo)
      : GenericDecl(TkInfo.Pos, AST_RangeDecl, "") {}

  const std::vector<RangeExpr *> &getRangeList() const { return Ranges; }
  void addRange(RangeExpr *Range) { Ranges.push_back(Range); }

  AST_NODE(RangeDecl)
};

class TypeDef : public GenericDecl {
  std::unique_ptr<GenericDecl> Type;

public:
  explicit TypeDef(const TokenInfo &TkInfo, TypeDef *Type = nullptr)
      : GenericDecl(TkInfo.Pos, AST_TypeDef, TkInfo.toString()), Type(Type) {}

  GenericDecl *getTypeDecl() { return Type.get(); }
  void setTypeDecl(GenericDecl *NewType) { Type.reset(NewType); }

  AST_NODE(TypeDef)
};

} // namespace north::ast

#endif // LIBNORTH_AST_DECLARATIONS_H
