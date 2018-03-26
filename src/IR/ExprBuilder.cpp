//===--- IR/ExprBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Diagnostic.h"
#include "IR/IRBuilder.h"
#include "Type/Type.h"
#include "Type/TypeInference.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO/FunctionImport.h>

#include <llvm/ADT/Twine.h>
#include <llvm/Support/FormatVariadic.h>
#include <llvm/Support/raw_ostream.h>

#define M Module.get()

namespace north::ir {

using namespace llvm;

Value *IRBuilder::visit(ast::UnaryExpr &Unary) {
  auto Expr = Unary.getOperand()->accept(*this);
  if (!Expr)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("invalid expression");

  /*
  if (Unary.getOperator() == Token::Minus) {
    auto Int = static_cast<ConstantInt *>(Expr);
    return ConstantInt::get(Int->getType(),
                            *Int->getValue().getRawData(), true);
  }
*/
  if (Unary.getOperator() == Token::Mult) {
    return Builder.CreateLoad(Expr);
  }

  return nullptr;
}

Value *IRBuilder::visit(ast::BinaryExpr &Expr) {
  GetVal = true;
  auto LHS = Expr.getLHS()->accept(*this);
  auto RHS = Expr.getRHS()->accept(*this);
  GetVal = false;

  if (!LHS || !RHS)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("invalid expression");

  switch (Expr.getOperator()) {
  case Token::Mult:
    return Builder.CreateMul(LHS, RHS);

  case Token::Div:
    return Builder.CreateSDiv(LHS, RHS);

  case Token::Plus:
    return Builder.CreateAdd(LHS, RHS);

  case Token::Minus:
    return Builder.CreateSub(LHS, RHS);

  case Token::LShift:
    return Builder.CreateShl(LHS, RHS);

  case Token::RShift:
    return Builder.CreateLShr(LHS, RHS);

  case Token::And:
    return Builder.CreateAnd(LHS, RHS);

  case Token::Or:
    return Builder.CreateOr(LHS, RHS);

  case Token::Eq:
    return Builder.CreateICmpEQ(LHS, RHS);

  case Token::NotEq:
    return Builder.CreateICmpNE(LHS, RHS);

  case Token::LessThan:
    return Builder.CreateICmpSLT(LHS, RHS);

  case Token::LessEq:
    return Builder.CreateICmpSLE(LHS, RHS);

  case Token::GreaterThan:
    return Builder.CreateICmpSGT(LHS, RHS);

  case Token::GreaterEq:
    return Builder.CreateICmpSGE(LHS, RHS);

  case Token::OrOr:
    return Builder.CreateOr(compareWithTrue(LHS), compareWithTrue(RHS));

  case Token::AndAnd:
    return Builder.CreateAnd(compareWithTrue(LHS), compareWithTrue(RHS));
  }

  llvm_unreachable("unsupported binary expression operator");
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
  }

  if (auto Var = CurrentScope->lookup(Token.toString())) {
    if (GetVal)
      return Builder.CreateLoad(Var->getIRValue());
    return Var->getIRValue();
  }

  Diagnostic(Module->getModuleIdentifier()).semanticError("unknown symbol");
}

Value *IRBuilder::visit(ast::RangeExpr &) { return nullptr; }

Value *IRBuilder::visit(ast::CallExpr &Callee) {
  auto Fn = Module->getFunction(Callee.getIdentifier());
  if (!Fn)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("unknown function referenced");

  if (Callee.numberOfArgs() != Fn->arg_size() && !Fn->isVarArg())
    Diagnostic(Module->getModuleIdentifier())
        .semanticError(formatv("expected {0} arguments, not {1}",
                               Callee.numberOfArgs(), Fn->arg_size()));

  std::vector<Value *> Args;
  if (Callee.hasArgs()) {
    Args.reserve(Callee.numberOfArgs() - 1);
    GetVal = true;
    for (auto &Arg : Callee.getArgumentList())
      Args.push_back(Arg->accept(*this));
    GetVal = false;
  }

  return Builder.CreateCall(Fn->getFunctionType(), Fn, Args);
}

llvm::Value *IRBuilder::visit(ast::ArrayIndexExpr &Idx) {
  auto Ty = Idx.getIdentifier()->accept(*this);
  auto Index = Idx.getIdxExpr()->accept(*this);

  auto Z = llvm::ConstantInt::get(Context, llvm::APInt(64, 0, true));
  auto I = Builder.CreateBitCast(Index, IntegerType::getInt64Ty(Context));

  auto Ptr = llvm::GetElementPtrInst::Create(
      Ty->getType(), Ty, {Z, I}, "",
      &CurrentFn->getIRValue()->getBasicBlockList().back());

  return Builder.CreateLoad(Ptr);
}

llvm::Value *IRBuilder::visit(ast::QualifiedIdentifierExpr &Ident) {
  auto Var = CurrentScope->lookup(Ident.getPart(0).toString());
  auto Struct = static_cast<ast::StructInitExpr *>(Var->getValue())->getType();

  unsigned I = 0;
  for (auto F : Struct->getFieldList()) {
    if (F->getIdentifier() == Ident.getPart(1).toString()) {
      auto Load = Builder.CreateLoad(Var->getIRValue());
      return Builder.CreateExtractValue(Load, {I});
    }
    ++I;
  }

  Diagnostic(Module->getModuleIdentifier())
      .semanticError("structure " + Struct->getIdentifier() +
                     "doesn't has field `" + Ident.getPart(1).toString() + "`");
}

Value *IRBuilder::visit(ast::IfExpr &If) {
  auto Cond = If.getExpr()->accept(*this);
  if (!Cond)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("empty if condition");

  Cond = compareWithTrue(Cond);

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

  auto Body = For.getBlock()->accept(*this);
  Value *StepVal = ConstantInt::get(Context, APInt(32, 1));
  auto NextVar = Builder.CreateAdd(Variable, StepVal, "nextvar");

  Value *Cond = Builder.CreateICmpNE(StartVal, EndVal, "loopcond");

  BasicBlock *LoopEndBB = Builder.GetInsertBlock();
  BasicBlock *AfterBB = BasicBlock::Create(Context, "afterloop", Fn);

  Builder.CreateCondBr(Cond, LoopBB, AfterBB);
  Builder.SetInsertPoint(AfterBB);

  Variable->addIncoming(NextVar, LoopEndBB);

  return Constant::getNullValue(Type::getInt32Ty(Context));
}

Value *IRBuilder::visit(ast::WhileExpr &While) { return nullptr; }

#define CREATE_ASSIGN(FN)                                                      \
  Builder.CreateStore(Builder.Create##FN(Builder.CreateLoad(LHS), RHS), LHS);

Value *IRBuilder::visit(ast::AssignExpr &Assign) {
  auto LHS = Assign.getLHS()->accept(*this),
       RHS = Assign.getRHS()->accept(*this);

  if (!LHS || !RHS)
    Diagnostic(Module->getModuleIdentifier())
        .semanticError("invalid assign expression");

  switch (Assign.getOperator()) {
  case Token::Assign:
    return Builder.CreateStore(RHS, LHS);

  case Token::DivAssign:
    return CREATE_ASSIGN(SDiv);

  case Token::MultAssign:
    return CREATE_ASSIGN(Mul);

  case Token::PlusAssign:
    return CREATE_ASSIGN(Add);

  case Token::MinusAssign:
    return CREATE_ASSIGN(Sub);

  case Token::RShiftAssign:
    return CREATE_ASSIGN(LShr);

  case Token::LShiftAssign:
    return CREATE_ASSIGN(Shl);

  case Token::AndAssign:
    return CREATE_ASSIGN(And);

  case Token::OrAssign:
    return CREATE_ASSIGN(Or);
  }

  llvm_unreachable("unsupported assign expression operator");
}

Value *IRBuilder::visit(ast::StructInitExpr &Struct) {
  auto Ty = getTypeFromIdent(Struct.getIdentifier());
  auto TypeDef = static_cast<ast::TypeDef *>(Ty->getDecl());
  auto StructDecl = static_cast<ast::StructDecl *>(TypeDef->getTypeDecl());
  Struct.setType(StructDecl);

  std::vector<Constant *> Fields;
  GetVal = true;
  for (auto Field : Struct.getValues())
    Fields.push_back(static_cast<Constant *>(Field->accept(*this)));
  GetVal = false;

  auto IR = ConstantStruct::get(static_cast<StructType *>(Ty->toIR(M)), Fields);
  Struct.setIRValue(IR);
  return IR;
}

Value *IRBuilder::visit(ast::ArrayExpr &Array) {
  auto FirstElemTy =
      type::inferExprType(Array.getValue(0), M, CurrentScope)->toIR(M);
  auto Ty = ArrayType::get(FirstElemTy, Array.getCap());

  std::vector<Constant *> Values;
  Values.reserve(Array.getCap());

  for (auto I : Array.getValues()) {
    auto Elem = I->accept(*this);
    if (Elem->getType() != FirstElemTy &&
        !CastInst::isCastable(Elem->getType(), FirstElemTy))
      Diagnostic(Module->getModuleIdentifier())
          .semanticError("array elements can't has different types");

    Values.push_back(static_cast<Constant *>(Elem));
  }

  return ConstantArray::get(Ty, Values);
}

} // namespace north::ir