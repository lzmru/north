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

#include "AST/AST.h"

#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>

namespace north::ast {
class GenericDecl;
} // namespace north::ast

namespace north::type {

class Module;
class Scope;

class Type {
  ast::GenericDecl *Decl;
  llvm::Type *IRType;
  Module *Mod;
  
  explicit Type(llvm::Type *T) : Decl(nullptr), IRType(T) {}

public:
  Type(ast::GenericDecl *TypeDecl, Module *Mod) : Decl(TypeDecl), IRType(nullptr), Mod(Mod) {
    assert(TypeDecl && "Can't generate IR without type declaration");
    assert(Mod && "Can't generate IR without target module");
  }

  llvm::Type *getIR();
  void setIR(llvm::Type *T);

  ast::GenericDecl *getDecl() { return Decl; }

  bool isPrimitive() { return !Decl; }

  static Type *Void;
  static Type *Int8;
  static Type *Int16;
  static Type *Int32;
  static Type *Int64;
  static Type *Float;
  static Type *Double;
  static Type *Char;

  static Type *getArrayType(Type *, uint64_t);
  
  friend Type *inferFunctionType(ast::FunctionDecl &, Module *, Scope *);
  friend Type *inferVarType(ast::VarDecl &, Module *, Scope *);
  friend Type *inferExprType(ast::Node *, Module *, Scope *);
};

} // namespace north::type

#endif // LIBNORTH_TYPE_TYPE_H
