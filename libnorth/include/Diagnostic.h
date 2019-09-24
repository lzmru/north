//===--- Diagnostic.h - Diagnostic provider ---------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_DIAGNOSTIC_H
#define LIBNORTH_DIAGNOSTIC_H

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

  void expectedToken(Token, const TokenInfo &);
  void unexpectedChar(Position);

  void invalidTypeDecl(const TokenInfo &);
  void invalidTupleDecl(const TokenInfo &);
  void invalidUnionDecl(const TokenInfo &);

  void invalidForExpr(const TokenInfo &);
  void invalidRangeExpr(const TokenInfo &);
  void invalidAssignExpr(const TokenInfo &);

  Diagnostic &semanticError(llvm::Twine);
  Diagnostic &semanticError(const Position &, llvm::Twine);

  Diagnostic &hint(llvm::Twine Hint);
};

} // namespace north

#endif // LIBNORTH_DIAGNOSTIC_H
