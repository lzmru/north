//===--- Diagnostic.cpp - Diagnostic provider -------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Diagnostic.h"

#include <llvm/ADT/Twine.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/FormatAdapters.h>
#include <llvm/Support/FormatVariadic.h>

using namespace llvm;

namespace north {

unsigned Diagnostic::ErrorCounter = 0;
unsigned Diagnostic::WarningCounter = 0;

namespace {

unsigned distanceToEnd(const Position &Pos) {
  unsigned Distance = 0;
  do {
    ++Distance;
    if (*(Pos.Offset + Distance) == '\0')
      break;
  } while (*(Pos.Offset + Distance) != '\n');

  return Distance - 1;
}

unsigned distanceToStart(const Position &Pos) {
  unsigned Distance = 0;
  do {
    ++Distance;
    if (*(Pos.Offset - Distance) == '\0')
      break;
  } while (*(Pos.Offset - Distance) != '\n');

  while (*(Pos.Offset - Distance + 1) == ' ')
    --Distance;

  return Distance - 1;
}

StringRef posToStr(const Position &Pos) {
  return formatv("{0}:{1}", Pos.Line, Pos.Column).str();
}

void printPath(StringRef Filename, const Position &Pos) {
  outs().changeColor(raw_ostream::BLUE, true)
      << formatv("{0}--> ", fmt_repeat(" ", 4));
  outs().resetColor() << Filename << ":" << posToStr(Pos) << "\n";
}

void printPath(StringRef Filename) {
  outs().changeColor(raw_ostream::SAVEDCOLOR) << Filename << ": ";
}

void printErrorMsg(Twine Msg) {
  outs().changeColor(raw_ostream::RED, true) << "error: ";
  outs().resetColor().changeColor(raw_ostream::SAVEDCOLOR, true);
  outs() << Msg;
}

void printWarningMsg(Twine Msg) {
  outs().changeColor(raw_ostream::MAGENTA, true) << "warning: ";
  outs().resetColor().changeColor(raw_ostream::SAVEDCOLOR, true);
  outs() << Msg;
}

void renderHighlightedDiagnostic(Position Pos, const char *Message = "") {
  outs().changeColor(raw_ostream::BLUE, true)
      << formatv("{0,=5}|\n{1,=5}| ", " ", Pos.Line);

  auto Start = distanceToStart(Pos), End = distanceToEnd(Pos);

  outs().resetColor() << "    " << StringRef(Pos.Offset - Start, Start);
  outs().changeColor(raw_ostream::RED, true)
      << StringRef(Pos.Offset, Pos.Length);
  outs().resetColor() << StringRef(Pos.Offset + Pos.Length, End);

  outs().changeColor(raw_ostream::BLUE, true) << formatv("\n{0,=5}|", " ");
  outs().changeColor(raw_ostream::RED, true)
      << formatv("{0}{1} {2}\n", fmt_repeat(" ", Start + 5),
                 fmt_repeat("^", Pos.Length), Message);
  outs().resetColor();
}

} // namespace

Diagnostic::~Diagnostic() {
  if (ErrorCounter)
    std::exit(0);
}

void Diagnostic::expectedToken(Token Expected, const TokenInfo &Found) {
  ++ErrorCounter;

  printErrorMsg(formatv("expected {0}, found {1}\n", tokenToString(Expected),
                        tokenToString(Found.Type)));
  printPath(Filename, Found.Pos);
  renderHighlightedDiagnostic(Found.Pos);
}

void Diagnostic::unexpectedChar(Position Found) {
  ++ErrorCounter;

  printPath(Filename, Found);
  printErrorMsg("unexpected char\n");
  renderHighlightedDiagnostic(Found);
}

void Diagnostic::invalidTypeDecl(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg("invalid type declaration\n");
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

void Diagnostic::invalidEnumDecl(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg(formatv("invalid enum declaration: expected `,`, found {0}",
                        TkInfo.toString()));
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

void Diagnostic::invalidTupleDecl(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg(
      formatv("invalid tuple declaration: unexpected {0}", TkInfo.toString()));
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

void Diagnostic::invalidUnionDecl(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg(
      formatv("invalid union declaration: unexpected {0}", TkInfo.toString()));
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

void Diagnostic::invalidForExpr(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg(
      formatv("invalid for expression: unexpected {0}", TkInfo.toString()));
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

void Diagnostic::invalidRangeExpr(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg(
      formatv("invalid range expression: unexpected {0}", TkInfo.toString()));
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

void Diagnostic::invalidAssignExpr(const TokenInfo &TkInfo) {
  ++ErrorCounter;

  printPath(Filename, TkInfo.Pos);
  printErrorMsg(
      formatv("invalid assign expression: unexpected {0}", TkInfo.toString()));
  renderHighlightedDiagnostic(TkInfo.Pos, "This");
}

Diagnostic &Diagnostic::semanticError(llvm::Twine Message) {
  ++ErrorCounter;

  printPath(Filename);
  printErrorMsg(Message);

  return *this;
}

Diagnostic &Diagnostic::semanticError(const Position &TkInfo,
                                      llvm::Twine Message) {
  ++ErrorCounter;

  printPath(Filename, TkInfo);
  printErrorMsg(Message);

  return *this;
}

Diagnostic &Diagnostic::hint(llvm::Twine Hint) {
  outs() << "\n\thint: " << Hint;
  return *this;
}

} // namespace north
