//===--- Parser.cpp - North compiler debug utils ----------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Trace.h"

unsigned Tabulator::Tab = 0;

Tabulator::Tabulator(llvm::raw_ostream &OS) {
  for (unsigned i = 0; i != Tab; ++i)
    OS << "  ";
  ++Tab;
}

Tabulator::~Tabulator() { --Tab; }
