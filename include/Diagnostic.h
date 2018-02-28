//===--- Diagnostic.h - Diagnostic provider ---------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_DIAGNOSTIC_H
#define NORTH_DIAGNOSTIC_H

#include "Grammar/Token.h"

namespace north {

class Diagnostic {
  static unsigned ErrorCounter, WarningCounter;
  llvm::StringRef Filename;

public:
  explicit Diagnostic(llvm::StringRef Filename) : Filename(Filename) {}
  ~Diagnostic();

  static unsigned getErrorNumber() { return ErrorCounter; }
  static unsigned getWarningNumber() { return WarningCounter; }

  void expectedToken(Token Expected, const TokenInfo &Found);
  void unexpectedChar(Position Found);

  void invalidTypeDecl(const TokenInfo &TkInfo);

  void invalidEnumDecl(const TokenInfo &TkInfo);
  void invalidTupleDecl(const TokenInfo &TkInfo);
  void invalidUnionDecl(const TokenInfo &TkInfo);

  void invalidForExpr(const TokenInfo &TkInfo);
  void invalidRangeExpr(const TokenInfo &TkInfo);
  void invalidAssignExpr(const TokenInfo &TkInfo);

  Diagnostic &semanticError(llvm::Twine Message);
  Diagnostic &semanticError(const Position &TkInfo, llvm::Twine Message);

  Diagnostic &hint(llvm::Twine Hint);
};

} // namespace north

#endif // NORTH_DIAGNOSTIC_H
