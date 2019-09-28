//===--- Grammar/Parser.cpp - North language parser -------------*- C++ -*-===//
//
//                     The North Compiler Infrastructure
//
//              This file is distributed under the MIT License.
//                      See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Grammar/Parser.h"

#include "AST/AST.h"

#include <llvm/IR/Function.h>
#include <llvm/Support/FormatVariadic.h>

namespace north {

using llvm::Function;
using llvm::FunctionType;
using llvm::Type;

Token Parser::nextToken() {
  if (!Peeked) {
    Buf[0] = Lex.getNextToken();
  } else {
    Buf[0] = Buf[1];
    Peeked = false;
  }
  return Buf[0].Type;
}

Token Parser::peekToken() {
  if (!Peeked) {
    Buf[1] = Lex.getNextToken();
    Peeked = true;
  }

  return Buf[1].Type;
}

bool Parser::match(Token With) {
  if (peekToken() == With) {
    nextToken();
    return true;
  }
  return false;
}

void Parser::expect(Token What) {
  if (!match(What)) {
    nextToken();

    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));
    auto Msg = llvm::formatv("expected {0}, found {1}\n", tokenToString(What),
                             tokenToString(Buf[0].Type));

    Lex.getSourceManager().PrintMessage(Range.Start,
        llvm::SourceMgr::DiagKind::DK_Error, Msg, Range);
  }
}

bool Parser::tryParseLiteral() {
  switch (peekToken()) {
  case Token::Int:
  case Token::Identifier:
  case Token::Char:
    nextToken();
    return true;

  default:
    return false;
  }
}

namespace {

enum Precedence {
  None = 0,
  Assign,  // = >>= <<= &= |= &^= ^= %=
  Cond,    // if
  OrOr,    // ||
  AndAnd,  // &&
  Eq,      // == !=
  Compare, // < > <= >=
  Op,      // + - | ^
  Binary,  // * | % << >> &
  Unary,   // ! * - & ++ --
  Call,    // () [] .
};

static const Precedence PrecedenceTable[] = {
    None, // Eof,
    None, // Comment,

    None, // Indent,
    None, // Dedent,

    None, // Identifier,
    None, // Int,
    None, // Char,
    None, // String,

    None, // Def,
    None, // Nil
    None, // Open,
    None, // Interface,
    None, // Type,
    None, // Var,
    None, // Let,
    Cond, // If,
    None, // In,
    None, // Else,
    None, // For,
    None, // While,
    None, // Switch,
    None, // Return,

    Call, // LParen,
    None, // RParen,

    Call, // LBrace,
    None, // RBrace,

    Call, // LBracket,
    None, // RBracket,

    Call, // Dot,
    None, // DotDot,
    None, // Ellipsis,

    Assign, // Assign,
    Assign, // DivAssign,
    Assign, // MultAssign,
    Assign, // PlusAssign,
    Assign, // MinusAssign,
    Assign, // AndAssign,
    Assign, // OrAssign,
    Assign, // RShiftAssign,
    Assign, // LShiftAssign,

    Eq,      // Eq,
    Eq,      // NotEq,
    Compare, // GreaterEq,
    Compare, // LessEq,

    None, // Colon,
    None, // Comma,
    None, // Semicolon,

    Binary, // Div,
    Binary, // Mult,
    Op,     // Plus,
    Op,     // Minus,

    Unary, // Increment,
    Unary, // Decrement,

    Unary,   // Not,
    Binary,  // And,
    Op,      // Or,
    Compare, // GreaterThan,
    Compare, // LessThan,
    None,    // Wildcard,

    AndAnd, // AndAnd,
    OrOr,   // OrOr,
    Binary, // RShift,
    Binary, // LShift,

    None, // RightArrow
};

uint8_t getTokenPrec(Token Tok) {
  return PrecedenceTable[static_cast<uint8_t>(Tok)];
}

} // namespace

/// toplevel = { openStmt
///            | typeDefinition
///            | functionDecl
///            | interfaceDecl
///            | varDecl };
void Parser::parse() {
  auto Tree = new llvm::simple_ilist<ast::Node>();

  while (true) {
    switch (nextToken()) {
    case Token::Open:
      Tree->push_back(*parseOpenStmt());
      break;

    case Token::Type:
      Tree->push_back(*parseTypeDefinition());
      break;

    case Token::Def:
      Tree->push_back(*parseFunctionDecl());
      break;

    case Token::Interface:
      Tree->push_back(*parseInterfaceDecl());
      break;

    case Token::Var:
      Tree->push_back(*parseVarDecl());
      break;

    case Token::Eof:
      Module->setAST(Tree);
      return;

    default:
      auto Pos = Buf[0].Pos;

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
          "unexpected char '" + llvm::StringRef(Pos.Offset, 1) + llvm::StringRef("\'"), Range);
    }
  }
}

/// openStmt = 'open' IDENTIFIER;
ast::OpenStmt *Parser::parseOpenStmt() {
  expect(Token::Identifier);
  return new ast::OpenStmt(Buf[0].Pos, Buf[0].toString());
}

/// typeDefinition =
///     'type' IDENTIFIER genericTypeList '='
///         ( aliasDecl
///         | structDecl
///         | unionDecl
///         | enumDecl
///         | tupleDecl
///         | rangeExpr );
ast::GenericDecl *Parser::parseTypeDefinition() {
  expect(Token::Identifier);
  auto Result = new ast::TypeDef(Buf[0]);

  parseGenericTypeList(Result);

  expect(Token::Assign);

  switch (nextToken()) {
  case Token::Mult:
    Result->setModifier(ast::GenericDecl::Modifier::Ptr);
    expect(Token::Identifier);

  case Token::Identifier:
    if (peekToken() == Token::Comma)
      Result->setTypeDecl(parseEnumDecl());
    else
      Result->setTypeDecl(parseAliasDecl());
    break;

  case Token::LBrace:
    Result->setTypeDecl(parseStructDecl());
    break;

  case Token::Or:
    Result->setTypeDecl(parseUnionDecl());
    break;

  case Token::Int:
  case Token::String:
    if (peekToken() == Token::DotDot)
      Result->setTypeDecl(parseRangeDecl());
    else
      Result->setTypeDecl(parseEnumDecl());
    break;

  case Token::LParen:
    Result->setTypeDecl(parseTupleDecl());
    break;

  default:
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "invalid type declaration", Range);
  }

  Module->addType(Result);
  return Result;
}

/// typeDecl = aliasDecl
///          | structDecl
///          | unionDecl
///          | tupleDecl;
ast::GenericDecl *Parser::parseTypeDecl() {
  switch (nextToken()) {
  case Token::Mult:
    expect(Token::Identifier);
    return parseAliasDecl(true);
  case Token::Identifier:
    return parseAliasDecl();

  case Token::LBrace:
    return parseStructDecl();

  case Token::Or:
    return parseUnionDecl();

  case Token::LParen:
    return parseTupleDecl();

  default:
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "invalid type declaration", Range);
  }

  return nullptr;
}

/// aliasDecl = IDENTIFIER { '.' IDENTIFIER } genericTypeList;
ast::AliasDecl *Parser::parseAliasDecl(bool IsPtr) {
  auto Alias = new ast::AliasDecl(Buf[0].Pos, Buf[0].toString());
  if (IsPtr)
    Alias->setModifier(ast::GenericDecl::Ptr);
  parseGenericTypeList(Alias);
  return Alias;
}

/// structDecl = '{' varDecl { ',' varDecl } '}';
ast::StructDecl *Parser::parseStructDecl() {
  auto Struct = new ast::StructDecl(Buf[0].Pos);

  while (auto Var = parseVarDecl()) {
    Struct->addField(Var);
    if (!match(Token::Comma))
      break;
  }

  expect(Token::RBrace);

  return Struct;
}

/// unionDecl = '|' IDENTIFIER { '|' IDENTIFIER };
ast::UnionDecl *Parser::parseUnionDecl() {
  auto Union = new ast::UnionDecl(Buf[0]);

  do {
    if (auto Type = parseTypeDecl()) {
      Union->addField(Type);
    } else {
      auto Pos = Buf[0].Pos;

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                          "invalid union declaration: unexpected " + Buf[0].toString(), Range);
    }
  } while (match(Token::Or));

  return Union;
}

/// enumDecl = (IDENTIFIER { ',' IDENTIFIER })
///          | rangeDecl;
ast::EnumDecl *Parser::parseEnumDecl() {
  auto Enum = new ast::EnumDecl(Buf[0]);

  // TODO: range decl in enum
  while (match(Token::Comma)) {
    expect(Token::Identifier);
    Enum->addMember(Buf[0]);
  }

  return Enum;
}

/// tupleDecl = '(' IDENTIFIER { ',' IDENTIFIER } ')';
ast::TupleDecl *Parser::parseTupleDecl() {
  auto Tuple = new ast::TupleDecl(Buf[0]);

  do {
    if (auto Member = parseVarDecl()) {
      Tuple->addMember(Member);
    } else {
      auto Pos = Buf[0].Pos;

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                          "invalid tuple declaration: unexpected " + Buf[0].toString(), Range);
    }
  } while (match(Token::Comma));

  expect(Token::RParen);

  return Tuple;
}

/// rangeDecl = 'type' IDENTIFIER '=' rangeExpr;
ast::RangeDecl *Parser::parseRangeDecl() {
  auto Ranges = new ast::RangeDecl(Buf[0]);
  auto CheckRange = [&] {
    if (tryParseLiteral() && (peekToken() == Token::DotDot))
      return true;
    return false;
  };

  do {
    Ranges->addRange(parseRangeExpr());
    if (!match(Token::Comma))
      break;
  } while (CheckRange());

  return Ranges;
}

/// interfaceDecl = 'interface' IDENTIFIER [genericTypeList]
///     [':' IDENTIFIER genericTypeList?] '='
///           functionSignature { '\n' functionSignature };
ast::InterfaceDecl *Parser::parseInterfaceDecl() {
  expect(Token::Identifier);
  auto Interface = new ast::InterfaceDecl(Buf[0]);
  parseGenericTypeList(Interface);

  if (match(Token::Colon)) {
    expect(Token::Identifier);
    auto Parent = new ast::InterfaceDecl(Buf[0]);
    parseGenericTypeList(Interface);
    Interface->setParent(Parent);
  }

  expect(Token::Assign);

  Lex.incrementIndentLevel();
  Lex.turnFlag(Lexer::IndentationSensitive, true);

  while (match(Token::Indent)) {
    expect(Token::Def);
    Interface->addFunction(parseFunctionSignature());
  }

  expect(Token::Dedent);
  Lex.decrementIndentLevel();
  Lex.turnFlag(Lexer::IndentationSensitive, false);

  return Interface;
}

ast::Node *Parser::parseExpression(uint8_t Prec) {
  nextToken();

  ast::Node *Result = parsePrefix();

  while (Prec < getTokenPrec(peekToken())) {
    nextToken();
    Result = parseInfix(Result);
  }

  return Result;
}

ast::Node *Parser::parsePrefix() {
  ast::Node *Res = nullptr;

  switch (Buf[0].Type) {
  case Token::Mult:
  case Token::Not:
  case Token::Minus:
  case Token::Increment:
  case Token::Decrement:
    return new ast::UnaryExpr(Buf[0], Buf[0].Type, parseExpression(Unary));

  case Token::Identifier:
    if (peekToken() == Token::Dot)
      return parseQualifiedIdentifier();

  case Token::Char:
  case Token::Int:
  case Token::String:
  case Token::Nil:
    return new ast::LiteralExpr(Buf[0]);

  case Token::If:
    return parseIfExpr();

  case Token::Else:
    if (!LastIfNode) {
      auto Pos = Buf[0].Pos;

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                          "else without if", Range);
    }
    LastIfNode->setElseBranch(parseIfExpr(true));
    LastIfNode = LastIfNode->getElseBranch();
    ExpectNull = true;
    return nullptr;

  case Token::Var:
    return parseVarDecl();

  case Token::For:
    return parseForExpr();

  case Token::While:
    return parseWhileExpr();

  case Token::LParen:
    Res = parseExpression();
    expect(Token::RParen);
    return Res;

  case Token::LBracket:
    return parseArrayExpr();

  default:
    return nullptr;
  }
}

ast::Node *Parser::parseInfix(ast::Node *LHS) {
  switch (Buf[0].Type) {
  case Token::LParen:
    return parseCallExpr(LHS);

  case Token::LBracket:
    return parseArrayIndexExpr(LHS);

  case Token::LBrace:
    return parseStructInitExpr(LHS);

  case Token::OrOr:
  case Token::AndAnd:
  case Token::Eq:
  case Token::NotEq:
  case Token::LessThan:
  case Token::LessEq:
  case Token::GreaterThan:
  case Token::GreaterEq:

  case Token::Mult:
  case Token::Div:
  case Token::LShift:
  case Token::RShift:
  case Token::And:
  case Token::Plus:
  case Token::Minus:
  case Token::Or:
    return new ast::BinaryExpr(Buf[0], LHS, Buf[0].Type,
                               parseExpression(getTokenPrec(Buf[0].Type)));

  case Token::Assign:
  case Token::DivAssign:
  case Token::MultAssign:
  case Token::PlusAssign:
  case Token::MinusAssign:
  case Token::AndAssign:
  case Token::OrAssign:
  case Token::RShiftAssign:
  case Token::LShiftAssign:
    return new ast::AssignExpr(Buf[0], LHS, Buf[0].Type,
                               parseExpression(getTokenPrec(Buf[0].Type)));

  default:
    return nullptr;
  }
}

/// structInitExpr = IDENTIFIER '{' expr { ',' expr } '}';
ast::StructInitExpr *Parser::parseStructInitExpr(ast::Node *Ident) {
  auto Expr = new ast::StructInitExpr(Ident);

  do {
    Expr->addValue(parseExpression());
  } while (match(Token::Comma));

  expect(Token::RBrace);
  return Expr;
}

/// callExpr = IDENTIFIER '(' expr { ',' expr } ')';
ast::CallExpr *Parser::parseCallExpr(ast::Node *Ident) {
  ast::CallExpr *Callee = nullptr;

  if (auto I = llvm::dyn_cast<ast::QualifiedIdentifierExpr>(Ident)) {
    Callee = new ast::CallExpr(I);
  } else if (auto L = llvm::dyn_cast<ast::LiteralExpr>(Ident)) {
    Callee =
        new ast::CallExpr(new ast::QualifiedIdentifierExpr(L->getTokenInfo()));
  } else {
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "invalid call expression", Range);
  }

  if (peekToken() != Token::RParen) {
    while (true) {
      if (match(Token::Identifier) && peekToken() == Token::Colon) {
        auto Name = Buf[0].toString();
        nextToken();
        Callee->addArgument(parseExpression(), Name);
      } else {
        Callee->addArgument(parseExpression());
      }

      if (!match(Token::Comma))
        break;
    }
  }

  expect(Token::RParen);

  return Callee;
}

/// arrayIndexExpr = IDENTIFIER '[' expr ']';
ast::ArrayIndexExpr *Parser::parseArrayIndexExpr(ast::Node *Ident) {
  auto Idx = new ast::ArrayIndexExpr(Ident);

  if (auto Expr = parseExpression()) {
    Idx->setIdxExpr(Expr);
  } else {
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "invalid array index", Range);
  }

  expect(Token::RBracket);
  return Idx;
}

/// qualifiedIdentifier = IDENTIFIER '.' IDENTIFIER { '.' IDENTIFIER };
ast::QualifiedIdentifierExpr *Parser::parseQualifiedIdentifier() {
  auto Ident = new ast::QualifiedIdentifierExpr(Buf[0]);

  while (match(Token::Dot)) {
    expect(Token::Identifier);
    Ident->AddPart(Buf[0]);
  }

  return Ident;
}

/// forExpr = 'for' literalExpr 'in' expr ':' blockStmt;
ast::ForExpr *Parser::parseForExpr() {
  auto *Loop = new ast::ForExpr(Buf[0]);

  // TODO: use parseExpression()
  if (tryParseLiteral()) {
    Loop->setIter(new ast::LiteralExpr(Buf[0]));
    expect(Token::In);

    if (tryParseLiteral()) {
      if (peekToken() == Token::DotDot)
        Loop->setRange(parseRangeExpr());
      else if (Buf[0].Type == Token::Identifier)
        Loop->setRange(new ast::LiteralExpr(Buf[0]));
      else
        goto __error;
    } else {
      goto __error;
    }

    nextToken();
    Loop->setBlock(parseBlockStmt());

    return Loop;
  } else {
  __error:
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "invalid for expression: unexpected " + Buf[0].toString(), Range);
    return nullptr;
  }
}

/// whileExpr = 'while' expr ':' blockStmt;
ast::WhileExpr *Parser::parseWhileExpr() {
  auto Loop = new ast::WhileExpr(Buf[0], parseExpression());
  expect(Token::Colon);
  Loop->setBlock(parseBlockStmt());

  return Loop;
}

/// ifExpr = 'if' expr ':' blockStmt
///        { 'else' ['if' expr] ':' blockStmt };
ast::IfExpr *Parser::parseIfExpr(bool isElse) {
  ast::IfExpr *If = nullptr;

  if (isElse) {
    If =
        new ast::IfExpr(Buf[0], match(Token::If) ? parseExpression() : nullptr);
  } else {
    If = new ast::IfExpr(Buf[0], parseExpression());
  }

  expect(Token::Colon);
  If->setBlock(parseBlockStmt());
  LastIfNode = If;

  return If;
}

/// rangeExpr = literalExpr '..' literalExpr;
ast::RangeExpr *Parser::parseRangeExpr() {
  auto Range = new ast::RangeExpr(Buf[0]);
  expect(Token::DotDot);

  if (tryParseLiteral()) {
    Range->setEndValue(new ast::LiteralExpr(Buf[0]));
  } else {
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "invalid range expression: unexpected " + Buf[0].toString(), Range);
  }

  return Range;
}

/// arrayExpr = '[' expr { ',' expr } ']';
ast::ArrayExpr *Parser::parseArrayExpr() {
  auto Array = new ast::ArrayExpr(Buf[0]);

  Lex.turnFlag(Lexer::IndentationSensitive, false);

  while (auto Val = parseExpression()) {
    Array->addValue(Val);
    if (!match(Token::Comma))
      break;
  }
  expect(Token::RBracket);

  if (!Array->getCap()) {
    auto Pos = Buf[0].Pos;

    auto Range = llvm::SMRange(
        llvm::SMLoc::getFromPointer(Pos.Offset),
        llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

    Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
                                        "unimplemented: empty array", Range);
  }

  Lex.turnFlag(Lexer::IndentationSensitive, true);

  return Array;
}

/// functionDecl = functionSignature [':' blockStmt];
ast::FunctionDecl *Parser::parseFunctionDecl() {
  auto Result = parseFunctionSignature();

  if (match(Token::Colon)) {
    auto Block = parseBlockStmt();
    Block->setOwner(Result);
    Result->setBlockStmt(Block);
  }

  Module->addFunction(Result);
  return Result;
}

/// functionSignature = 'def' IDENTIFIER [genericTypeList] argumentList
///         ['->' typeDecl];
ast::FunctionDecl *Parser::parseFunctionSignature() {
  expect(Token::Identifier);
  auto Signature = new ast::FunctionDecl(Buf[0]);

  parseGenericTypeList(Signature);
  parseArgumentList(Signature);

  if (match(Token::RightArrow))
    Signature->setTypeDecl(parseTypeDecl());

  return Signature;
}

/// arg = ( WILDCARD | IDENTIFIER );
/// argumentList = '(' { arg? varDecl { ',' arg? varDecl } } ['...'] ')';
void Parser::parseArgumentList(ast::FunctionDecl *Function) {
  expect(Token::LParen);
  if (match(Token::RParen))
    return;

  do {
    if (match(Token::Ellipsis)) {
      Function->setVarArg(true);
      break;
    }


    Function->addArgument(parseVarDecl(true));
  } while (match(Token::Comma));

  expect(Token::RParen);
}

// TODO: labelStmt, breakStmt, continueStmt
/// primary = expr
///         | returnStmt;
ast::Node *Parser::parsePrimary() {
  if (match(Token::Return)) {
    auto ReturnStmt = new ast::ReturnStmt(Buf[0].Pos);
    ReturnStmt->setReturnExpr(parseExpression());
    return ReturnStmt;
  }

  if (auto Expr = parseExpression())
    return Expr;

  return nullptr;
}

/// blockStmt = INDENT primary { '\n' INDENT primary };
ast::BlockStmt *Parser::parseBlockStmt() {
  Lex.turnFlag(Lexer::IndentationSensitive, true);

  Lex.incrementIndentLevel();

  auto Block = new ast::BlockStmt(Buf[0].Pos, CurrentBlock);
  CurrentBlock = Block;

  while (match(Token::Indent)) {
    if (auto Primary = parsePrimary()) {
      Block->addNode(Primary);
    } else if (ExpectNull) {
      ExpectNull = false;
    } else {
      break;
    }
  }

  expect(Token::Dedent);

  Lex.decrementIndentLevel();

  if (!Lex.getIndentLevel()) {
    Lex.turnFlag(Lexer::IndentationSensitive, false);
    LastIfNode = nullptr;
  }

  return Block;
}

/// varDecl = [WILDCARD | IDENTIFIER] IDENTIFIER [':' typeDecl] ['=' expr];
ast::VarDecl *Parser::parseVarDecl(bool IsArg) {
  if (!match(Token::Identifier) && !match(Token::Wildcard)) {
    if (IsArg) {
      auto Pos = Buf[0].Pos;

      auto Range = llvm::SMRange(
          llvm::SMLoc::getFromPointer(Pos.Offset),
          llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));

      Lex.getSourceManager().PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
          "expected `identifier` or `_`, found " + Buf[0].toString(), Range);
    } else {
      return nullptr;
    }
  }

  auto Result = new ast::VarDecl(Buf[0], IsArg);

  if (IsArg) {
    auto Buffer = Buf[0];
    if (match(Token::Identifier)) {
      Result->setNamedArg(Buffer.toString());
      Result->setIdentifier(Buf[0].toString());
    }
  }

  if (match(Token::Colon))
    Result->setType(parseTypeDecl());
  if (match(Token::Assign))
    Result->setValue(parseExpression());

  return Result;
}

/// genericTypeList = [ '[' genericType { ',' genericType } ']' ];
void Parser::parseGenericTypeList(ast::GenericDecl *Declaration) {
  if (!match(Token::LBracket))
    return;

  do {
    parseGenericType(Declaration);
  } while (match(Token::Comma));

  expect(Token::RBracket);
}

/// genericType = IDENTIFIER;
/// TODO: specialization
void Parser::parseGenericType(ast::GenericDecl *Declaration) {
  expect(Token::Identifier);
  Declaration->addGenericType(Buf[0].Pos, Buf[0].toString());
}

} // namespace north
