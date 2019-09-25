//===--- Grammar/Lexer.h - North language lexer -----------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_LEXER_H
#define LIBNORTH_LEXER_H

#include "Token.h"

#include <bitset>
#include <llvm/Support/SourceMgr.h>

namespace north {

class Lexer {
  llvm::SourceMgr& SourceManager;

  const char *Buffer;
  const char *BufferEnd;

  Position Pos;
  std::bitset<2> Flags;
  uint8_t IndentLevel = 0;
  bool NewLine = false;

public:
  enum LexerFlag {
    YieldComments = 0,
    IndentationSensitive = 1,
  };

  explicit Lexer(llvm::SourceMgr&);

  void turnFlag(LexerFlag F, bool State) { Flags[F] = State; }
  bool getFlagState(LexerFlag F) { return Flags[F]; }

  TokenInfo getNextToken();

  void incrementIndentLevel() { ++IndentLevel; }
  void decrementIndentLevel() { --IndentLevel; }
  uint8_t getIndentLevel() { return IndentLevel; }

  const llvm::SourceMgr& getSourceManager() const { return SourceManager; }

private:
  void skipWhitespace();
  Token keywordOrIdentifier();
  TokenInfo makeToken(Token Type);
  TokenInfo makeToken(Token Type, uint8_t Length);
  TokenInfo makeEof();
  uint8_t checkIndentLevel();
};

} // namespace north

#endif // LIBNORTH_LEXER_H
