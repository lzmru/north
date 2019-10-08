//===--- Grammar/Parser.h - North language parser ---------------*- C++ -*-===//
//
//                     The North Compiler Infrastructure
//
//              This file is distributed under the MIT License.
//                      See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_PARSER_H
#define LIBNORTH_PARSER_H

#include "AST/AST.h"
#include "Lexer.h"
#include "Type/Module.h"

namespace north {

namespace ast {

class ASTNode;
class OpenStmt;
class FunctionDecl;
class TypeDecl;
class GenericDecl;

} // namespace ast

class Parser {
  Lexer& Lex;
  bool Peeked = false;
  TokenInfo Buf[2];

  ast::BlockStmt *CurrentBlock = nullptr;
  type::Module *Module = nullptr;
  bool ExpectNull = false;
  ast::IfExpr *LastIfNode = nullptr;

public:
  explicit Parser(Lexer& Lexer, type::Module* Module)
      : Lex(Lexer), Module(Module) {}

  void parse();

private:
  Token nextToken();
  Token peekToken();
  bool match(Token With);
  void expect(Token What);

  bool tryParseLiteral();

  ast::OpenStmt *parseOpenStmt();

  ast::GenericDecl *parseTypeDefinition();
  ast::GenericDecl *parseTypeDecl();
  ast::AliasDecl *parseAliasDecl(bool IsPtr = false);
  ast::StructDecl *parseStructDecl();
  ast::UnionDecl *parseUnionDecl();
  ast::EnumDecl *parseEnumDecl();
  ast::TupleDecl *parseTupleDecl();
  ast::RangeDecl *parseRangeDecl();
  ast::InterfaceDecl *parseInterfaceDecl();

  ast::Node *parseExpression(uint8_t Prec = 0, bool SkipCurrentToken = true);
  ast::Node *parsePrefix();
  ast::Node *parseInfix(ast::Node *);

  ast::StructInitExpr *parseStructInitExpr(ast::Node *);
  ast::CallExpr *parseCallExpr(ast::Node *Ident);
  ast::ArrayIndexExpr *parseArrayIndexExpr(ast::Node *);
  ast::QualifiedIdentifierExpr *parseQualifiedIdentifier();
  ast::IfExpr *parseIfExpr(bool isElse = false);
  ast::ForExpr *parseForExpr();
  ast::WhileExpr *parseWhileExpr();
  ast::RangeExpr *parseRangeExpr();
  ast::ArrayExpr *parseArrayExpr();

  ast::FunctionDecl *parseFunctionDecl();
  ast::FunctionDecl *parseFunctionSignature();
  void parseArgumentList(ast::FunctionDecl *);

  ast::Node *parsePrimary();
  ast::BlockStmt *parseBlockStmt();

  ast::VarDecl *parseVarDecl(bool IsArg = false);

  void parseGenericTypeList(ast::GenericDecl *);
  void parseGenericType(ast::GenericDecl *);
};

} // namespace north

#endif // LIBNORTH_PARSER_H
