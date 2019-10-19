//===--- Dumper/Dumper.cpp — AST dumping implementation ---------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Dumper.h"

#include "AST/AST.h"
#include "Trace.h"

#include <llvm/Support/Error.h>
#include <llvm/Support/raw_ostream.h>

namespace {

using namespace llvm;
using namespace north::ast;

class NodePrinter {
  static int Tab;
  bool In;
  static bool Out;
  static bool OnlyData;

public:
  explicit NodePrinter(StringRef NodeName, const north::ast::Node &Node,
                       bool InIndent = true) {
    In = InIndent;

    if (Out)
      indent();
    ++Tab;

    if (OnlyData)
      return;

    outs().changeColor(raw_ostream::MAGENTA, true);
    outs() << NodeName;
    outs().resetColor() << ": ";
    printPos(Node.getPosition());
    outs().resetColor() << " {" << (In ? "\n" : " ");
  }

  explicit NodePrinter(StringRef NodeName, bool InIndent = true) {
    In = InIndent;

    if (Out)
      indent();
    ++Tab;

    if (OnlyData)
      return;

    outs().changeColor(raw_ostream::YELLOW, true);
    outs() << NodeName;
    outs().resetColor() << ": {" << (In ? "\n" : " ");
  }

  ~NodePrinter() {
    --Tab;
    Out = true;
    if (OnlyData) {
      OnlyData = false;
      return;
    }
    if (In)
      indent();
    outs() << (!In ? " }\n" : "}\n");
  }

  void offOutIndent() { Out = false; }
  void onOutIndent() { Out = true; }

  void printOnlyData() { OnlyData = true; }

  void indent() { outs().indent(Tab * 2); }

  raw_ostream &printField(const char *Name) {
    if (In)
      indent();
    outs().changeColor(raw_ostream::YELLOW, true) << Name;
    outs().resetColor() << ": ";
    return outs();
  }

  void printGenericList(const GenericDecl &Decl) {
    NodePrinter Node("Generics");
    for (const auto &Generic : Decl.getGenericsList()) {
      indent();
      outs() << Generic.Name << ' ';
      printPos(Generic.Pos) << ",\n";
    }
  }

  void printArgumentList(FunctionDecl &Func) {
    NodePrinter Node("Arguments");
    Dumper Dump;

    for (auto Arg : Func.getArgumentList()) {
      Node.indent();
      printIdentifier(Arg->getIdentifier()) << ' ';
      printPos(Arg->getPosition()) << " -> ";

      Node.offOutIndent();
      Arg->getType()->accept(Dump);
    }
  }

  void printArgumentList(CallExpr &Func) {
    NodePrinter Node("Arguments");

    for (auto Arg : Func.getArgumentList()) {
      Dumper Dump;
      Arg->Arg->accept(Dump);
    }
  }

  raw_ostream &printPos(const north::Position &Pos) {
    outs().resetColor() << '(';
    outs().changeColor(raw_ostream::RED) << Pos.Line;
    outs().resetColor() << ':';
    outs().changeColor(raw_ostream::RED) << Pos.Column;
    outs().resetColor() << ", ";
    outs().changeColor(raw_ostream::RED) << Pos.Length;
    outs().resetColor() << ')';
    return outs();
  }

  raw_ostream &printIdentifier(const StringRef Identifier) {
    outs().changeColor(raw_ostream::CYAN) << Identifier;
    return outs().resetColor();
  }
};

int NodePrinter::Tab = 0;
bool NodePrinter::Out = true;
bool NodePrinter::OnlyData = false;

} // namespace

namespace north::ast {

Value *Dumper::visit(FunctionDecl &Func) {
  NodePrinter Node("FunctionDecl", Func);

  Node.printField("Name");
  Node.printIdentifier(Func.getIdentifier()) << ",\n";

  if (auto Type = Func.getTypeDecl()) {
    Node.printField("Type");
    Node.offOutIndent();
    Type->accept(*this);
    // Node.printIdentifier(Type->getIdentifier()) << ",\n";
  }

  if (Func.hasGenerics())
    Node.printGenericList(Func);
  if (Func.hasArgs())
    Node.printArgumentList(Func);

  if (auto Block = Func.getBlockStmt())
    visit(*Block);

  return nullptr;
}
  
Value *Dumper::visit(ast::GenericFunctionDecl &Fn) {
  return nullptr;
}

Value *Dumper::visit(InterfaceDecl &Interface) {
  NodePrinter Node("InterfaceDecl", Interface);

  Node.printField("Name") << Interface.getIdentifier() << '\n';
  Node.printGenericList(Interface);

  if (auto Parent = Interface.getParent()) {
    Node.printField("Parent");
    Node.offOutIndent();
    Parent->accept(*this);
  }

  for (const auto &Signature : Interface.getDemands()) {
    Signature->accept(*this);
  }

  return nullptr;
}

Value *Dumper::visit(VarDecl &Var) {
  NodePrinter Node("VarDecl", Var);

  Node.printField("Name") << Var.getIdentifier() << ",\n";
  
  if (auto Type = Var.getType()) {
    Node.printField("Type");
    Node.offOutIndent();
    Type->accept(*this);
  }

  if (auto Value = Var.getValue()) {
    Node.printField("Value");
    Node.offOutIndent();
    Value->accept(*this);
  }

  return nullptr;
}

Value *Dumper::visit(AliasDecl &Alias) {
  NodePrinter Node("AliasDecl", Alias, Alias.hasGenerics());

  if (Alias.hasGenerics())
    Node.printField("Name");
  Node.printIdentifier(Alias.getIdentifier());
  if (Alias.hasGenerics()) {
    outs() << '\n';
    Node.indent();
    Node.printGenericList(Alias);
  }

  return nullptr;
}

Value *Dumper::visit(StructDecl &Struct) {
  NodePrinter Node("StructDecl", Struct);

  for (const auto &Field : Struct.getFieldList())
    Field->accept(*this);

  return nullptr;
}

Value *Dumper::visit(EnumDecl &Enum) {
  NodePrinter Node("EnumDecl", Enum);

  Node.indent();
  for (const auto &Member : Enum.getMemberList())
    Node.printIdentifier(tokenView(Member.getTokenInfo())) << ", ";
  outs() << '\n';

  if (Enum.hasGenerics())
    Node.printGenericList(Enum);

  return nullptr;
}

Value *Dumper::visit(UnionDecl &Union) {
  NodePrinter Node("UnionDecl", Union);

  for (auto Member : Union.getFieldList())
    Member->accept(*this);

  return nullptr;
}

Value *Dumper::visit(TupleDecl &Tuple) {
  NodePrinter Node("TupleDecl", Tuple);

  for (auto Member : Tuple.getMemberList())
    Member->accept(*this);

  return nullptr;
}

Value *Dumper::visit(RangeDecl &Ranges) {
  NodePrinter Node("RangeDecl", Ranges);

  for (auto Range : Ranges.getRangeList())
    Range->accept(*this);

  return nullptr;
}

Value *Dumper::visit(TypeDef &Def) {
  NodePrinter Node("TypeDef", Def);
  Node.printField("Identifier");

  Node.printIdentifier(Def.getIdentifier()) << ",\n";
  if (Def.hasGenerics())
    Node.printGenericList(Def);

  Def.getTypeDecl()->accept(*this);

  return nullptr;
}

Value *Dumper::visit(UnaryExpr &Unary) {
  NodePrinter Node("UnaryExpr", Unary);

  Node.indent();
  Node.printIdentifier(tokenToString(Unary.getOperator())) << '\n';
  Unary.getOperand()->accept(*this);

  return nullptr;
}

Value *Dumper::visit(BinaryExpr &Binary) {
  NodePrinter Node("BinaryExpr", Binary);

  Node.indent();
  Node.printIdentifier(tokenToString(Binary.getOperator())) << "\n";
  Node.onOutIndent();
  Binary.getLHS()->accept(*this);
  Binary.getRHS()->accept(*this);

  return nullptr;
}

Value *Dumper::visit(LiteralExpr &Literal) {
  NodePrinter Node("LiteralExpr", Literal, false);
  Node.printIdentifier(Literal.getTokenInfo().toString());

  return nullptr;
}

Value *Dumper::visit(RangeExpr &Range) {
  NodePrinter Node("RangeExpr", Range, false);

  Node.printField("From") << Range.getBeginValue()->getTokenInfo().toString()
                          << ", ";
  Node.printField("To") << Range.getEndValue()->getTokenInfo().toString();

  return nullptr;
}

Value *Dumper::visit(CallExpr &Callee) {
  NodePrinter Node("CallExpr", Callee);

  Node.printField("Name");
  Node.offOutIndent();
  Callee.getIdentifier()->accept(*this);

  if (Callee.hasArgs())
    Node.printArgumentList(Callee);

  return nullptr;
}

Value *Dumper::visit(ArrayIndexExpr &Index) {
  NodePrinter Node("ArrayIndexExpr", Index);
  Node.printField("Identifier");
  Node.printOnlyData();
  Index.getIdentifier()->accept(*this);
  llvm::outs() << "\n";
  Node.printField("Index");
  Node.offOutIndent();
  Index.getIdxExpr()->accept(*this);
  return nullptr;
}

Value *Dumper::visit(QualifiedIdentifierExpr &Ident) {
  NodePrinter Node("QualifiedIdentifierExpr", Ident, false);

  auto Identifier = Ident.getIdentifier();
  int I = 0, E = Identifier.size();
  for (auto PartOfIdent : Identifier) {
    outs().changeColor(raw_ostream::CYAN) << PartOfIdent.toString();
    if (++I != E)
      outs().resetColor() << '.';
  }

  outs().resetColor();
  return nullptr;
}

Value *Dumper::visit(IfExpr &If) {
  NodePrinter Node("IfExpr", If);
  Node.offOutIndent();

  if (auto Expr = If.getExpr()) {
    Node.printField("Expr");
    Expr->accept(*this);
  }

  Node.onOutIndent();
  if (auto Block = If.getBlock())
    Block->accept(*this);

  if (auto ElseBranch = If.getElseBranch()) {
    if (ElseBranch->getExpr())
      Node.printField("ElseIfExpr");
    else
      Node.printField("ElseExpr");
    Node.offOutIndent();
    ElseBranch->accept(*this);
  }

  return nullptr;
}

Value *Dumper::visit(ForExpr &For) {
  NodePrinter Node("ForExpr", For);

  Node.printField("Iter");
  Node.offOutIndent();
  For.getIter()->accept(*this);

  Node.printField("Range");
  Node.offOutIndent();
  For.getRange()->accept(*this);
  For.getBlock()->accept(*this);

  return nullptr;
}

Value *Dumper::visit(WhileExpr &While) {
  NodePrinter Node("WhileExpr", While);

  Node.printField("Expr");
  Node.offOutIndent();
  While.getExpr()->accept(*this);
  While.getBlock()->accept(*this);

  return nullptr;
}

Value *Dumper::visit(AssignExpr &Assign) {
  NodePrinter Node("AssignExpr", Assign);

  Node.indent();
  Node.printIdentifier(tokenToString(Assign.getOperator())) << '\n';
  if (auto LHS = Assign.getLHS())
    LHS->accept(*this);
  if (auto RHS = Assign.getRHS())
    RHS->accept(*this);

  return nullptr;
}

Value *Dumper::visit(ast::StructInitExpr &Struct) {
  NodePrinter Node("StructInitExpr", Struct);
  Node.onOutIndent();

  for (auto FieldVal : Struct.getValues())
    FieldVal->accept(*this);

  return nullptr;
}

Value *Dumper::visit(ast::ArrayExpr &Array) {
  NodePrinter Node("ArrayExpr", Array);
  Node.indent();

  for (auto Val : Array.getValues())
    Val->accept(*this);

  return nullptr;
}

Value *Dumper::visit(OpenStmt &Stmt) {
  NodePrinter Node("OpenStmt", Stmt);

  Node.printField("Module");
  Node.printIdentifier(Stmt.getModuleName());
  outs() << '\n';

  return nullptr;
}

Value *Dumper::visit(BlockStmt &Block) {
  NodePrinter Node("BlockStmt", Block);

  if (auto Body = Block.getBody()) {
    for (auto CurrentLine = Body->begin(), End = Body->end();
         CurrentLine != End; ++CurrentLine) {
      CurrentLine->accept(*this);
    }
  }

  return nullptr;
}

Value *Dumper::visit(ReturnStmt &Stmt) {
  NodePrinter Node("ReturnStmt", Stmt);

  if (auto Expr = Stmt.getReturnExpr())
    Expr->accept(*this);

  return nullptr;
}

} // namespace north::ast
