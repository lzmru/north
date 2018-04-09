//===--- Grammar/Token.h - North token-related utils ------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_TOKEN_H
#define NORTH_TOKEN_H

#include <llvm/ADT/StringRef.h>

namespace north {

enum class Token {
  Eof = 0,
  Comment,

  Indent,
  Dedent,

  Identifier,
  Int,
  Char,
  String,

  Def,
  Nil,
  Open,
  Interface,
  Type,
  Var,
  Let,
  If,
  In,
  Else,
  For,
  While,
  Switch,
  Return,

  LParen,
  RParen,

  LBrace,
  RBrace,

  LBracket,
  RBracket,

  Dot,
  DotDot,
  Ellipsis,

  Assign,
  DivAssign,
  MultAssign,
  PlusAssign,
  MinusAssign,
  AndAssign,
  OrAssign,
  RShiftAssign,
  LShiftAssign,

  Eq,
  NotEq,
  GreaterEq,
  LessEq,

  Colon,
  Comma,
  Semicolon,

  Div,
  Mult,
  Plus,
  Minus,

  Increment,
  Decrement,

  Not,
  And,
  Or,
  GreaterThan,
  LessThan,
  Wildcard,

  AndAnd,
  OrOr,
  RShift,
  LShift,

  RightArrow,
};

struct Position {
  unsigned Line;
  unsigned Column;
  const char *Offset;
  unsigned Length;
};

struct TokenInfo {
  Position Pos;
  Token Type;

  llvm::StringRef toString() const;
};

const char *tokenToString(Token Tk);
llvm::StringRef tokenView(TokenInfo Tk);

} // namespace north

#endif // NORTH_TOKEN_H
