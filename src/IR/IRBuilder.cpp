//===--- IR/IRBuilder.cpp - Transformation AST to LLVM IR -------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "IR/IRBuilder.h"
#include "Diagnostic.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>

namespace north::ir {

#define M Module.get()

using namespace llvm;

llvm::LLVMContext IRBuilder::Context;

Value *IRBuilder::visit(ast::FunctionDecl &Fn) {
  if (!Fn.getBlockStmt())
    return nullptr;
  auto BB = BasicBlock::Create(Context, "entry", Fn.getIRValue());
  Builder.SetInsertPoint(BB);
  // inferFunctionType(Fn, M);
  return Fn.getBlockStmt()->accept(*this);
}

Value *IRBuilder::visit(ast::InterfaceDecl &) { return nullptr; }

Value *IRBuilder::visit(ast::VarDecl &Var) {
  // TODO: Register local variables for tracing
  // if (IndentLevel) {
  //
  // }

  llvm::Type *Type = nullptr;

  if (auto TypeDecl = Var.getType()) {
    Type = Module->getType(Var.getType()->getIdentifier())->toIR(M);
    if (TypeDecl->isPtr())
      Type = Type->getPointerTo(0);
  } else {
    Type = inferVarType(Var, M)->toIR(M);
    Var.setIRType(Type);
  }

  auto IR = Builder.CreateAlloca(Type, nullptr, Var.getIdentifier());
  Var.setIRValue(IR);
  CurrentScope->addElement(&Var);

  if (auto Val = Var.getValue())
    Builder.CreateStore(Val->accept(*this), IR);

  return nullptr;
}

Value *IRBuilder::visit(ast::AliasDecl &Alias) {
  return nullptr;
  // return llvm::GlobalAlias::create(Builder.getVoidTy(), 0,
  //                                 GlobalValue::LinkageTypes::ExternalLinkage,
  //                                 Alias.getIdentifier(), M);
}

Value *IRBuilder::visit(ast::StructDecl &Struct) {
  std::vector<llvm::Type *> Args;
  for (auto Field : Struct.getFieldList()) {
    auto Ident = Field->getType()->getIdentifier();
    Args.push_back(Module->getType(Ident)->toIR(M));
  }
  Struct.getIR()->setBody(Args);
  return nullptr;
}

Value *IRBuilder::visit(ast::EnumDecl &Enum) {
  // TODO: check and generate
  return nullptr;
}

Value *IRBuilder::visit(ast::UnionDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::TupleDecl &) { return nullptr; }
Value *IRBuilder::visit(ast::RangeDecl &) { return nullptr; }

Value *IRBuilder::visit(ast::TypeDef &Type) {
  return Type.getTypeDecl()->accept(*this);
}

Value *IRBuilder::visit(ast::UnaryExpr &Unary) {
  auto Expr = Unary.getOperand()->accept(*this);
  if (!Expr)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("invalid expression");

  if (Unary.getOperator() == Token::Minus) {
    auto OldExpr = static_cast<ConstantInt *>(Expr);
    return ConstantInt::get(OldExpr->getType(),
                            *OldExpr->getValue().getRawData(), true);
  }

  return nullptr;
}

Value *IRBuilder::visit(ast::BinaryExpr &Expr) {
  auto LHS = Expr.getLHS()->accept(*this);
  auto RHS = Expr.getRHS()->accept(*this);

  if (!LHS || !RHS)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("invalid expression");

  switch (Expr.getOperator()) {
  case Token::Plus:
    return Builder.CreateAdd(LHS, RHS, "addtmp");

  case Token::Minus:
    return Builder.CreateSub(LHS, RHS, "subtmp");

  case Token::Mult:
    return Builder.CreateMul(LHS, RHS, "multmp");

  case Token::LessThan:
    return Builder.CreateICmpULT(LHS, RHS, "cmptmp");
    // return Builder.CreateUIToFP(LHS, Type::getDoubleTy(Context), "booltmp");
  }

  return nullptr;
}

Value *IRBuilder::visit(ast::LiteralExpr &Literal) {
  auto Token = Literal.getTokenInfo();

  switch (Token.Type) {
  case Token::Char:
    return ConstantInt::get(IntegerType::getInt8Ty(Context),
                            static_cast<uint64_t>(Token.toString().front()));
  case Token::String:
    return Builder.CreateGlobalStringPtr(Token.toString());
  case Token::Int:
    return ConstantInt::get(Context, APInt(32, Token.toString(), 10));
  case Token::Identifier:
    return CurrentScope->lookup(Token.toString())->getIRValue();
  }

  return nullptr;
}

Value *IRBuilder::visit(ast::RangeExpr &) { return nullptr; }

Value *IRBuilder::visit(ast::CallExpr &Callee) {
  Function *Function = Module->getFunction(Callee.getIdentifier());
  if (!Function)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("unknown function referenced");

  if (Callee.numberOfArgs() != Function->arg_size() && !Function->isVarArg())
    Diagnostic(Module->getModuleIdentifier())
        .semanticError(formatv("expected {0} arguments, not {1}",
                               Callee.numberOfArgs(), Function->arg_size()));

  std::vector<Value *> Args;
  Args.reserve(Callee.numberOfArgs() - 1);
  for (auto &Arg : Callee.getArgumentList())
    Args.push_back(Arg->accept(*this));

  return Builder.CreateCall(Function->getFunctionType(), Function, Args,
                            "calltmp");
}

Value *IRBuilder::visit(ast::IfExpr &If) {
  auto Cond = If.getExpr()->accept(*this);
  if (!Cond)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("empty if condition");

  Cond = Builder.CreateICmp(
      llvm::CmpInst::ICMP_NE, Cond,
      ConstantInt::get(Type::getInt1Ty(Context), 0, false), "ifcond");

  Function *Fn = Builder.GetInsertBlock()->getParent();

  BasicBlock *ThenBB = BasicBlock::Create(Context, "then", Fn);
  BasicBlock *ElseBB = BasicBlock::Create(Context, "else");
  BasicBlock *MergeBB = BasicBlock::Create(Context, "ifcont");

  Builder.CreateCondBr(Cond, ThenBB, ElseBB);

  Builder.SetInsertPoint(ThenBB);

  if (!If.getBlock())
    Diagnostic(Module->getModuleIdentifier()).semanticError("empty if block");
  auto Then = If.getBlock()->accept(*this);

  Builder.CreateBr(MergeBB);
  ThenBB = Builder.GetInsertBlock();

  // Emit else block.
  Fn->getBasicBlockList().push_back(ElseBB);
  Builder.SetInsertPoint(ElseBB);

  Value *Else = Builder.CreateICmp(
      llvm::CmpInst::ICMP_NE, Cond,
      ConstantInt::get(Type::getInt1Ty(Context), 1, false), "ifcond");
  if (auto ElseBlock = If.getElseBranch()->getBlock())
    ElseBlock->accept(*this);
  else
    Diagnostic(Module->getModuleIdentifier()).semanticError("empty else block");

  Builder.CreateBr(MergeBB);
  ElseBB = Builder.GetInsertBlock();

  Fn->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);

  PHINode *PN = Builder.CreatePHI(Type::getInt1Ty(Context), 2, "iftmp");

  PN->addIncoming(Cond, ThenBB);
  PN->addIncoming(Else, ElseBB);

  return PN;
}

Value *IRBuilder::visit(ast::ForExpr &For) {
  auto Range = static_cast<ast::RangeExpr *>(For.getRange());
  auto StartVal = Range->getBeginValue()->accept(*this);
  auto EndVal = Range->getEndValue()->accept(*this);
  auto Iter = static_cast<ast::LiteralExpr *>(For.getIter());

  if (!StartVal || !EndVal)
    Diagnostic(Module->getModuleIdentifier()).semanticError("invalid range");

  auto Fn = Builder.GetInsertBlock()->getParent();
  auto PreheaderBB = Builder.GetInsertBlock();
  auto LoopBB = BasicBlock::Create(Context, "loop", Fn);

  Builder.CreateBr(LoopBB);
  Builder.SetInsertPoint(LoopBB);

  auto VarName = Iter->getTokenInfo().toString();
  PHINode *Variable = Builder.CreatePHI(Type::getInt32Ty(Context), 2, VarName);
  Variable->addIncoming(StartVal, PreheaderBB);

  // Emit the body of the loop. This, like any other expr, can change the
  // current BB.  Note that we ignore the value computed by the body, but don't
  // allow an error.
  auto Body = For.getBlock()->accept(*this);
  Value *StepVal = ConstantInt::get(Context, APInt(32, 1));
  auto NextVar = Builder.CreateAdd(Variable, StepVal, "nextvar");

  Value *Cond = Builder.CreateICmpNE(StartVal, EndVal, "loopcond");

  // Create the "after loop" block and insert it.
  BasicBlock *LoopEndBB = Builder.GetInsertBlock();
  BasicBlock *AfterBB = BasicBlock::Create(Context, "afterloop", Fn);

  // Insert the conditional branch into the end of LoopEndBB.
  Builder.CreateCondBr(Cond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  Builder.SetInsertPoint(AfterBB);

  // Add a new entry to the PHI node for the backedge.
  Variable->addIncoming(NextVar, LoopEndBB);

  // for expr always returns 0.0.
  return Constant::getNullValue(Type::getInt32Ty(Context));
}

Value *IRBuilder::visit(ast::WhileExpr &) { return nullptr; }
Value *IRBuilder::visit(ast::AssignExpr &) {
  llvm::outs() << "assignExpr\n";
  return nullptr;
}

Value *IRBuilder::visit(ast::StructInitExpr &Struct) {
  auto Ty = type::Type::Int8->toIR(M);
  Constant *AllocSize =
      ConstantExpr::getSizeOf(Module->getType(Struct.getTypeName())->toIR(M));
  // AllocSize = ConstantExpr::getTruncOrBitCast(
  //    AllocSize, Type::Int64->toIR(M));

  auto FTy =
      FunctionType::get(Ty->getPointerTo(0), type::Type::Int64->toIR(M), false);

  return Builder.CreateCall(Module->getOrInsertFunction("malloc", FTy),
                            AllocSize);
}

Value *IRBuilder::visit(ast::ArrayExpr &Array) {
  auto ElemTy = type::inferExprType(Array.getValue(0), M)->toIR(M);
  auto Ty = ArrayType::get(ElemTy, Array.getCap());

  std::vector<Constant *> Values;
  Values.reserve(Array.getCap());
  for (auto I : Array.getValues())
    Values.push_back(
        static_cast<Constant *>(I->accept(*this))); // FIXME: check types

  return ConstantArray::get(Ty, Values);
}

Value *IRBuilder::visit(ast::OpenStmt &O) {
  // FunctionImporter FI(M->getProfileSummary(), );
  return nullptr;
}

Value *IRBuilder::visit(ast::BlockStmt &Block) {
  type::Scope Scope(CurrentScope, M);
  CurrentScope = &Scope;

  Value *Result = nullptr;

  if (auto Body = Block.getBody()) {
    for (auto I = Body->begin(), E = Body->end(); I != E; ++I)
      Result = I->accept(*this);
  }

  CurrentScope = Scope.getParent();
  return Result;
}

Value *IRBuilder::visit(ast::ReturnStmt &Return) {
  // TODO: type checking
  if (auto Expr = Return.getReturnExpr())
    return Builder.CreateRet(Expr->accept(*this));
  return Builder.CreateRetVoid();
}

} // namespace north::ir
