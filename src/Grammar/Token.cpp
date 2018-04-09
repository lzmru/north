//===--- Grammar/Token.cpp - North token-related utils ----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Grammar/Token.h"

namespace north {

llvm::StringRef TokenInfo::toString() const {
  if (*Pos.Offset == '\'' || *Pos.Offset == '\"')
    return llvm::StringRef(Pos.Offset + 1, Pos.Length - 2);
  return llvm::StringRef(Pos.Offset, Pos.Length);
}

const char *tokenToString(Token Tk) {
  static const char *Tokens[] = {
      "end of file", "comment",     "indent",   "dedent", "identifier",
      "`int`",       "char",        "`string`", "`def`",  "`nil`",
      "`open`",      "`interface`", "`type`",   "`var`",  "`let`",
      "`if`",        "`in`",        "`else`",   "`for`",  "`while`",
      "`switch`",    "`return`",    "`(`",      "`)`",    "`{`",
      "`}`",         "`[`",         "`]`",      "`.`",    "`..`",
      "`...`",       "`=`",         "`/=`",     "`*=`",   "`+=`",
      "`-=`",        "`&=`",        "`|=`",     "`>>=`",  "`<<=`",
      "`==`",        "`!=`",        "`>=`",     "`<=`",   "`:`",
      "`,`",         "`;`",         "`/`",      "`*`",    "`+`",
      "`-`",         "`++`",        "`--`",     "`!`",    "`&`",
      "`|`",         "`>`",         "`<`",      "`_`",    "`&&`",
      "`||`",        "`>>`",        "`<<`",     "`->`"};

  return Tokens[static_cast<uint8_t>(Tk)];
}

llvm::StringRef tokenView(TokenInfo Tk) {
  return llvm::StringRef(Tk.Pos.Offset, Tk.Pos.Length);
}

} // namespace north
