//===--- Type/Module.h - North module representation ------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_TYPE_MODULE_H
#define LIBNORTH_TYPE_MODULE_H

#include "AST/AST.h"

#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/ilist.h>
#include <llvm/IR/Module.h>
#include <llvm/ADT/TinyPtrVector.h>
#include <llvm/Support/SourceMgr.h>

namespace north::ast {

class GenericDecl;
class InterfaceDecl;
class FunctionDecl;

} // namespace north::ast

namespace north::type {

class Scope;
class Type;

class Module : public llvm::Module {
  using InterfaceDecl     = north::ast::InterfaceDecl;
  using InterfaceListType = llvm::StringMap<InterfaceDecl *>;
  using TypeListType      = llvm::StringMap<Type *>;
  using FunctionListType  = llvm::StringMap<north::ast::FunctionDecl *>;
  using ImportListType    = std::vector<llvm::StringRef>;

  std::unique_ptr<Scope> GlobalScope;
  InterfaceListType InterfaceList;
  TypeListType TypeList;
  FunctionListType FunctionList;
  ImportListType ImportList;

  llvm::simple_ilist<ast::Node> *AST;

  llvm::SourceMgr& SourceManager;

public:
  explicit Module(llvm::StringRef, llvm::LLVMContext &, llvm::SourceMgr &);

  Type *getType(llvm::StringRef Name) const;
  Type *getTypeOrNull(llvm::StringRef Name) const;
  InterfaceDecl *getInterface(llvm::StringRef Name) const;
  ast::FunctionDecl * getFn(ast::CallExpr &Callee, Scope *S);
  const ImportListType& getImportList() const { return ImportList; }

  void addType(north::ast::GenericDecl *);
  void addInterface(north::ast::InterfaceDecl *);
  void addFunction(north::ast::FunctionDecl *);
  void addImport(north::ast::OpenStmt *);

  Scope *getGlobalScope() { return GlobalScope.get(); }

  void setAST(llvm::simple_ilist<ast::Node> *NewAST) { AST = NewAST; }
  llvm::simple_ilist<ast::Node> *getAST() { return AST; }

  llvm::SourceMgr &getSourceManager() const { return SourceManager; }
};

} // namespace north::type

#endif // LIBNORTH_TYPE_MODULE_H
