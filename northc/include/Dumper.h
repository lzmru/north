//===--- Dumper/Dumper.h — AST dumping implementation -----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_AST_DUMPER_H
#define NORTH_AST_DUMPER_H

#include "AST/Visitor.h"

namespace north::ast {

class Dumper : public Visitor {
public:
  AST_WALKER_METHODS
};

} // namespace north::ast

#endif // NORTH_AST_DUMPER_H
