//===--- Grammar/Parser.h - North language parser ---------------*- C++ -*-===//
//
//                     The North Compiler Infrastructure
//
//              This file is distributed under the MIT License.
//                      See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_PARSER_H
#define NORTH_PARSER_H

#include "AST/AST.h"
#include "Grammar/Lexer.h"
#include "IR/IRBuilder.h"
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
  const char *Filename;
  Lexer Lex;
  bool Peeked;
  TokenInfo Buf[2];

  ast::BlockStmt *CurrentBlock = nullptr;
  type::Module *Module = nullptr;
  bool ExpectNull = false;
  ast::IfExpr *LastIfNode = nullptr;

public:
  explicit Parser(const char *Filename)
      : Filename(Filename), Lex(Filename), Peeked(false),
        Module(new type::Module(Filename, ir::IRBuilder::getContext())) {}

  north::type::Module *parse();

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

  ast::Node *parseExpression(uint8_t Prec = 0);
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

  ast::VarDecl *parseVarDecl();

  void parseGenericTypeList(ast::GenericDecl *);
  void parseGenericType(ast::GenericDecl *);
};

} // namespace north

#endif // NORTH_PARSER_H
