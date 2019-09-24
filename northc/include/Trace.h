//===--- Trace.h - North compiler debug utils -------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_TRACE_H
#define NORTH_TRACE_H

#include <llvm/Support/raw_ostream.h>

class Tabulator {
  static unsigned Tab;

public:
  explicit Tabulator(llvm::raw_ostream &OS);
  ~Tabulator();
};

#ifdef NDEBUG // DEBUG_TRACING // TODO

#include <llvm/Support/Debug.h>

#define TRACE()                                                                \
  Tabulator t(llvm::dbgs());                                                   \
  llvm::dbgs() << __FUNCTION__ << '\n';

#else

#define TRACE()

#endif // DEBUG_TRACING

#endif // NORTH_TRACE_H
