//===--- Grammar/Lexer.cpp - North language lexer ---------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

/// WHITESPACE = { '\n' | '\t' | '\r' | ' ' };
/// KEYWORD = 'def' | 'else' | 'for' | 'if'
///         | 'in' | 'interface' | 'let' | 'open'
///         | 'return' | 'switch' | 'type' | 'var'
///         | 'while';
/// INDENT = '\n  ';
/// CHAR = 'a' ... 'z' | 'A' ... 'Z';
/// SYMBOL = DIGIT | CHAR | '_';
/// IDENTIFIER = CHAR | '_' { SYMBOL };

#include "Grammar/Lexer.h"

namespace north {

namespace {

struct Keyword {
  const char *Keyword;
  unsigned char Length;
  Token KwToken;
};

#define KW_COUNT 14
const Keyword Keywords[KW_COUNT] = {
    {"def", 3, Token::Def},       {"else", 4, Token::Else},
    {"for", 3, Token::For},       {"if", 2, Token::If},
    {"in", 2, Token::In},         {"interface", 9, Token::Interface},
    {"let", 3, Token::Let},       {"nil", 3, Token::Nil},
    {"open", 4, Token::Open},     {"return", 6, Token::Return},
    {"switch", 6, Token::Switch}, {"type", 4, Token::Type},
    {"var", 3, Token::Var},       {"while", 5, Token::While}};

} // namespace

Lexer::Lexer(llvm::SourceMgr& SourceMgr) : SourceManager(SourceMgr) {
  Buffer = SourceManager.getMemoryBuffer(1)->getBufferStart();
  BufferEnd = SourceManager.getMemoryBuffer(1)->getBufferEnd();

  Pos.Offset = Buffer;
  Pos.Column = 1;
  Pos.Line = 1;
}

void Lexer::skipWhitespace() {
  if (NewLine)
    return;

  while (true) {
    switch (*Pos.Offset) {
    case '\n':
    __new_line:
      ++Pos.Offset;
      ++Pos.Line;

      if (*Pos.Offset == '\n')
        goto __new_line;

      Pos.Column = 1;

      if (getFlagState(IndentationSensitive)) {
        NewLine = true;
        return;
      }

      continue;

    case ' ':
    case '\t':
    case '\r':
      ++Pos.Offset;
      ++Pos.Column;
      continue;

    default:
      return;
    }
  }
}

Token Lexer::keywordOrIdentifier() {
  if (Pos.Length < 2 || Pos.Length > 9)
    return Token::Identifier;

  for (uint8_t i = 0; i != KW_COUNT; ++i) {
    if (Pos.Length == Keywords[i].Length) {
      if (strncmp(Keywords[i].Keyword, Pos.Offset, Pos.Length) == 0)
        return Keywords[i].KwToken;
    }
  }
  return Token::Identifier;
}

TokenInfo Lexer::makeToken(Token Tok) {
  TokenInfo result{Pos, Tok};
  Pos.Column += Pos.Length;
  Pos.Offset += Pos.Length;
  return result;
}

TokenInfo Lexer::makeToken(Token Tok, uint8_t Length) {
  Pos.Length = Length;
  TokenInfo Result{Pos, Tok};
  Pos.Column += Pos.Length;
  Pos.Offset += Pos.Length;
  return Result;
}

TokenInfo Lexer::makeEof() {
  if (IndentLevel)
    return TokenInfo{Pos, Token::Dedent};
  return TokenInfo{Pos, Token::Eof};
}

uint8_t Lexer::checkIndentLevel() {
  uint8_t I = 0, NecessaryIndent = IndentLevel * 2;

  for (; I != NecessaryIndent; ++I)
    if (Pos.Offset[I] != ' ')
      return 0;

  return I;
}

TokenInfo Lexer::getNextToken() {
  __start:

  skipWhitespace();

  if (NewLine) {
    if (auto Length = checkIndentLevel()) {
      NewLine = false;
      if (getFlagState(IndentationSensitive))
        return makeToken(Token::Indent, Length);
      Pos.Length = Length;
      Pos.Column += Length;
      Pos.Offset += Length;
    } else {
      if (IndentLevel)
        return makeToken(Token::Dedent, 0);

      NewLine = false;
    }
    goto __start;
  }

  Pos.Length = 0;

  if (isalpha(*Pos.Offset)) {
    while (isalnum(Pos.Offset[++Pos.Length]) || Pos.Offset[Pos.Length] == '_')
      ;

    return makeToken(keywordOrIdentifier());
  }

  if (isdigit(*Pos.Offset)) {
    while (isdigit(Pos.Offset[++Pos.Length]))
      ;

    return makeToken(Token::Int);
  }

  switch (*Pos.Offset) {
  case '"': {
    do {
      ++Pos.Length;
      if (Pos.Offset[Pos.Length] == '\0')
        return makeEof();
    } while (Pos.Offset[Pos.Length] != '"');

    ++Pos.Length;
    return makeToken(Token::String);
  }

  case '\'':
    return makeToken(Token::Char, 3);

  case '_': {
    if (Pos.Offset[1] == '_' || isalpha(Pos.Offset[1])) {
      do {
        ++Pos.Length;
      } while (Pos.Offset[Pos.Length] == '_' ||
          isalnum(Pos.Offset[Pos.Length]));

      return makeToken(keywordOrIdentifier());
    }
    return makeToken(Token::Wildcard, 1);
  }

  case '#':
    do {
      ++Pos.Length;
      if (Pos.Offset[Pos.Length] == '\0')
        return makeEof();
    } while (Pos.Offset[Pos.Length] != '\n');

    ++Pos.Length;
    ++Pos.Line;

    if (Flags[YieldComments]) {
      return makeToken(Token::Comment);
    } else {
      Pos.Column += Pos.Length;
      Pos.Offset += Pos.Length;
      goto __start;
    }

  case '/': {
    if (Pos.Offset[1] == '=')
      return makeToken(Token::DivAssign, 2);
    return makeToken(Token::Div, 1);
  }

  case '(':
    return makeToken(Token::LParen, 1);
  case ')':
    return makeToken(Token::RParen, 1);

  case '[':
    return makeToken(Token::LBracket, 1);
  case ']':
    return makeToken(Token::RBracket, 1);

  case '{':
    return makeToken(Token::LBrace, 1);
  case '}':
    return makeToken(Token::RBrace, 1);

  case '.': {
    if (Pos.Offset[1] == '.') {
      if (Pos.Offset[2] == '.')
        return makeToken(Token::Ellipsis, 3);
      return makeToken(Token::DotDot, 2);
    }
    return makeToken(Token::Dot, 1);
  }

  case '=': {
    if (Pos.Offset[1] == '=')
      return makeToken(Token::Eq, 2);
    return makeToken(Token::Assign, 1);
  }

  case ':':
    return makeToken(Token::Colon, 1);
  case ',':
    return makeToken(Token::Comma, 1);
  case ';':
    return makeToken(Token::Semicolon, 1);

  case '*':
    if (Pos.Offset[1] == '=')
      return makeToken(Token::MultAssign, 2);
    return makeToken(Token::Mult, 1);
  case '+':
    if (Pos.Offset[1] == '=')
      return makeToken(Token::PlusAssign, 2);
    if (Pos.Offset[1] == '+')
      return makeToken(Token::Increment, 2);
    return makeToken(Token::Plus, 1);
  case '-':
    if (Pos.Offset[1] == '=')
      return makeToken(Token::MinusAssign, 2);
    if (Pos.Offset[1] == '>')
      return makeToken(Token::RightArrow, 2);
    if (Pos.Offset[1] == '-')
      return makeToken(Token::Decrement, 2);
    return makeToken(Token::Minus, 1);

  case '&': {
    if (Pos.Offset[1] == '&')
      return makeToken(Token::AndAnd, 2);
    if (Pos.Offset[1] == '=')
      return makeToken(Token::AndAssign, 2);
    return makeToken(Token::And, 1);
  }

  case '|': {
    if (Pos.Offset[1] == '|')
      return makeToken(Token::OrOr, 2);
    if (Pos.Offset[1] == '=')
      return makeToken(Token::OrAssign, 2);
    return makeToken(Token::Or, 1);
  }

  case '>': {
    if (Pos.Offset[1] == '>') {
      if (Pos.Offset[2] == '=')
        return makeToken(Token::RShiftAssign, 3);
      return makeToken(Token::RShift, 2);
    }
    if (Pos.Offset[1] == '=')
      return makeToken(Token::GreaterEq, 2);
    return makeToken(Token::GreaterThan, 1);
  }

  case '<': {
    if (Pos.Offset[1] == '<') {
      if (Pos.Offset[2] == '=')
        return makeToken(Token::LShiftAssign, 3);
      return makeToken(Token::LShift, 2);
    }
    if (Pos.Offset[1] == '=')
      return makeToken(Token::LessEq, 2);
    return makeToken(Token::LessThan, 1);
  }

  case '!':
    if (Pos.Offset[1] == '=')
      return makeToken(Token::NotEq, 2);
    return makeToken(Token::Not, 1);
  }

  if (*Pos.Offset == '\0' || Pos.Offset >= BufferEnd)
    return makeEof();


  auto Range = llvm::SMRange(
      llvm::SMLoc::getFromPointer(Pos.Offset),
      llvm::SMLoc::getFromPointer(Pos.Offset + Pos.Length));
  SourceManager.PrintMessage(Range.Start, llvm::SourceMgr::DiagKind::DK_Error,
      "unexpected char '" + *Pos.Offset + '\'', Range);

  return {};
}

} // namespace north
