//===--- Type/Module.h - North module representation ------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTH_TYPE_MODULE_H
#define NORTH_TYPE_MODULE_H

#include "AST/AST.h"

#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/ilist.h>
#include <llvm/IR/Module.h>

namespace north::ast {

class GenericDecl;
class InterfaceDecl;

} // namespace north::ast

namespace north::type {

class Scope;
class Type;

class Module : public llvm::Module {
  using InterfaceDecl = north::ast::InterfaceDecl;

  using InterfaceListType = llvm::StringMap<InterfaceDecl *>;
  using TypeListType = llvm::StringMap<Type *>;

  std::unique_ptr<Scope> GlobalScope;
  InterfaceListType InterfaceList;
  TypeListType TypeList;

  llvm::simple_ilist<ast::Node> *AST;

public:
  explicit Module(llvm::StringRef ModuleID, llvm::LLVMContext &C);

  Type *getType(llvm::StringRef Name) const;
  Type *getTypeOrNull(llvm::StringRef Name) const;
  InterfaceDecl *getInterface(llvm::StringRef Name) const;
  llvm::Function *getFn(ast::CallExpr &, Scope *);

  void addType(north::ast::GenericDecl *);
  void addInterface(north::ast::InterfaceDecl *);
  void addFunction(north::ast::FunctionDecl *);

  Scope *getGlobalScope() { return GlobalScope.get(); }

  void setAST(llvm::simple_ilist<ast::Node> *NewAST) { AST = NewAST; }
  llvm::simple_ilist<ast::Node> *getAST() { return AST; }
};

} // namespace north::type

#endif // NORTH_TYPE_MODULE_H
