//===--- Grammar/Lexer.h - North language lexer -----------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_LEXER_H
#define NORTH_LEXER_H

#include "Token.h"

namespace north {

class Lexer {
  const char *Filename;
  const char *Buffer;
  const char *BufferEnd;
  Position Pos;
  uint8_t Flags;
  uint8_t IndentLevel;
  bool NewLine;

public:
  enum LexerFlag {
    YieldComments = 1,
    IndentationSensitive = 2, // 4, 8
  };

  explicit Lexer(const char *Path);
  void switchFlag(LexerFlag Flag);
  TokenInfo getNextToken();

  void incrementIndentLevel() { ++IndentLevel; }
  void decrementIndentLevel() { --IndentLevel; }
  uint8_t getIndentLevel() { return IndentLevel; }

private:
  void skipWhitespace();
  Token keywordOrIdentifier();
  TokenInfo makeToken(Token Type);
  TokenInfo makeToken(Token Type, uint8_t Length);
  uint8_t checkIndentLevel();
};

} // namespace north

#endif // NORTH_LEXER_H
