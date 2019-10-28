//===--- Dumper.h — AST dumping implementation ------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTHC_AST_DUMPER_H
#define NORTHC_AST_DUMPER_H

#include "AST/Visitor.h"

#include <llvm/Support/raw_ostream.h>

namespace north::ast {

class Dumper : public Visitor {
public:
  AST_WALKER_METHODS
};

} // namespace north::ast

#endif // NORTHC_AST_DUMPER_H
