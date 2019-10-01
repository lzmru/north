//===--- Type/Type.h - North language type representation -------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_TYPE_TYPE_H
#define LIBNORTH_TYPE_TYPE_H

#include <llvm/IR/Value.h>

namespace north::ast {
class GenericDecl;
} // namespace north::ast

namespace north::type {

class Module;

class Type {
  std::unique_ptr<ast::GenericDecl> Decl;
  llvm::Type *IRValue;

public:
  explicit Type(ast::GenericDecl *TypeDecl, Module *Mod)
      : Decl(TypeDecl), IRValue(nullptr) {}
  explicit Type(llvm::Type *Value) : Decl(nullptr), IRValue(Value) {}

  llvm::Type *toIR(Module *M);
  void setIR(llvm::Type *Value) { IRValue = Value; }

  ast::GenericDecl *getDecl() { return Decl.get(); }

  bool isPrimitive() { return !Decl.get(); }

  static Type *Void;
  static Type *Int;
  static Type *Int8;
  static Type *Int16;
  static Type *Int32;
  static Type *Int64;
  static Type *Int128;
  static Type *Float;
  static Type *Double;
  static Type *String;
  static Type *Char;

  static Type *getArrayType(Type *, uint64_t);
};

} // namespace north::type

#endif // LIBNORTH_TYPE_TYPE_H
