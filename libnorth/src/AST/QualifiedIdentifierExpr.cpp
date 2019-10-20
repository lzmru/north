//===--- AST/QualifiedIdentifierExpr.cpp ------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/AST.h"

namespace north::ast {
  
bool QualifiedIdentifierExpr::operator==(const QualifiedIdentifierExpr &RHS) const {
  if (this->getSize() == RHS.getSize()) {
    for (size_t I = 0; I < this->getSize(); ++I)
      if (this->getPart(I) != RHS.getPart(I))
        return false;
    return true;
  }
  return false;
}
  
} // namespace north::ast
