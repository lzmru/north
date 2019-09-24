//===--- IR/ExprBuilder.cpp - Transformation AST to LLVM IR -----*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../../../include/Diagnostic.h"
#include "Targets/IRBuilder.h"
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

namespace north::targets {

using namespace llvm;

Value *IRBuilder::visit(ast::UnaryExpr &Unary) {
  auto Expr = Unary.getOperand()->accept(*this);
  if (!Expr)
    Diagnostic(Module->getSourceFileName())
        .semanticError(Unary.getPosition(), "invalid expression");

  switch (Unary.getOperator()) {
  case Token::Mult:
    return Builder.CreateLoad(Expr);

  case Token::Not:
    return Builder.CreateNot(Expr);

  case Token::Increment:
    return Builder.CreateStore(
        Builder.CreateAdd(
            Builder.CreateLoad(Expr),
            ConstantInt::get(Type::getInt32Ty(Context), 1, false)),
        Expr);

  case Token::Decrement:
    return Builder.CreateStore(
        Builder.CreateSub(
            Builder.CreateLoad(Expr),
            ConstantInt::get(Type::getInt32Ty(Context), 1, false)),
        Expr);

  case Token::Minus:
    return Builder.CreateNeg(Expr);

  default:
    llvm_unreachable("unsupported unary expression operator");
  }
}

#define BINARY(FN) return Builder.Create##FN(LHS, RHS);

Value *IRBuilder::visit(ast::BinaryExpr &Expr) {
  GetVal = true;
  auto LHS = Expr.getLHS()->accept(*this);
  auto RHS = Expr.getRHS()->accept(*this);

  if (!LHS || !RHS)
    Diagnostic(Module->getSourceFileName())
        .semanticError(Expr.getPosition(), "invalid expression");

  switch (Expr.getOperator()) {
  case Token::Mult:
    BINARY(Mul);

  case Token::Div:
    BINARY(SDiv);

  case Token::Plus:
    if (LHS->getType()->isPointerTy())
      return Builder.CreateGEP(LHS, {RHS});
    BINARY(Add);

  case Token::Minus:
    BINARY(Sub);

  case Token::LShift:
    BINARY(Shl);

  case Token::RShift:
    BINARY(LShr);

  case Token::And:
    BINARY(And);

  case Token::Or:
    BINARY(Or);

  case Token::Eq:
    BINARY(ICmpEQ);

  case Token::NotEq:
    BINARY(ICmpNE);

  case Token::LessThan:
    BINARY(ICmpSLT);

  case Token::LessEq:
    BINARY(ICmpSLE);

  case Token::GreaterThan:
    BINARY(ICmpSGT);

  case Token::GreaterEq:
    BINARY(ICmpSGE);

  case Token::OrOr:
    return Builder.CreateOr(cmpWithTrue(LHS), cmpWithTrue(RHS));

  case Token::AndAnd:
    return Builder.CreateAnd(cmpWithTrue(LHS), cmpWithTrue(RHS));

  default:
    llvm_unreachable("unsupported binary expression operator");
  }
}

Value *IRBuilder::visit(ast::LiteralExpr &Literal) {
  auto Token = Literal.getTokenInfo();

  switch (Token.Type) {
  case Token::Nil:
    return Constant::getNullValue(Type::getInt32Ty(Context));

  case Token::Char:
    return ConstantInt::get(IntegerType::getInt8Ty(Context),
                            static_cast<uint64_t>(Token.toString().front()));
  case Token::String:
    return Builder.CreateGlobalStringPtr(Token.toString());

  case Token::Int:
    return ConstantInt::get(Context, APInt(32, Token.toString(), 10));

  case Token::Identifier:
    if (auto Var = CurrentScope->lookup(Token.toString()))
      return GetVal && !Var->isArg() &&
                     (LoadArg && !isa<StructType>(Var->getIRType()) &&
                      !isa<ArrayType>(Var->getIRType()))
                 ? Builder.CreateLoad(Var->getIRValue())
                 : Var->getIRValue();

    Diagnostic(Module->getSourceFileName())
        .semanticError(Literal.getPosition(),
                       "unknown symbol `" + Token.toString() + "`");

  default:
    return nullptr;
  }
}

Value *IRBuilder::visit(ast::RangeExpr &) { return nullptr; }

Value *IRBuilder::visit(ast::CallExpr &Callee) {
  auto Fn = Module->getFn(Callee, CurrentScope);

  if (!Fn)
    Diagnostic(Module->getSourceFileName())
        .semanticError(Callee.getPosition(), "unknown function referenced");

  if (Callee.numberOfArgs() != Fn->arg_size() && !Fn->isVarArg())
    Diagnostic(Module->getSourceFileName())
        .semanticError(Callee.getPosition(),
                       formatv("expected {0} arg, not {1}", Fn->arg_size(),
                               Callee.numberOfArgs()));

  std::vector<Value *> Args;
  if (Callee.hasArgs()) {
    Args.reserve(Callee.numberOfArgs() - 1);
    LoadArg = true;
    for (auto &Arg : Callee.getArgumentList()) {
      GetVal = true;
      auto Val = Arg->accept(*this);

      if (Val->getType()->isPointerTy()) {
        if (auto Arr =
                dyn_cast<ArrayType>(Val->getType()->getPointerElementType()))
          Val = Builder.CreateBitCast(Val,
                                      Arr->getElementType()->getPointerTo(0));
      }

      Args.push_back(Val);
    }
    LoadArg = false;
    GetVal = false;
  }

  auto IR = Builder.CreateCall(Fn->getFunctionType(), Fn, Args);
  Callee.setIR(IR);
  return IR;
}

llvm::Value *IRBuilder::visit(ast::ArrayIndexExpr &Idx) {
  GetVal = false;
  auto Ty = Idx.getIdentifier()->accept(*this);
  auto Index = Idx.getIdxExpr()->accept(*this);

  if (auto Store = dyn_cast<StoreInst>(Index))
    Index = Store->getOperand(1);
  if (Index->getType()->isPointerTy())
    Index = Builder.CreateLoad(Index);

  Value *GEP = Builder.CreateInBoundsGEP(Ty, {Index});
  while (GEP->getType()->isPointerTy())
    GEP = Builder.CreateLoad(GEP);

  return GEP;
}

llvm::Value *IRBuilder::visit(ast::QualifiedIdentifierExpr &Ident) {
  auto FirstPart = Ident.getPart(0);

  if (auto Var = CurrentScope->lookup(FirstPart))
    return getStructField(Var->getValue() ? Var->getValue() : Var,
                          Var->getIRValue(), Ident);

  if (auto Type = Module->getTypeOrNull(FirstPart)) {
    auto T = static_cast<ast::TypeDef *>(Type->getDecl())->getTypeDecl();
    if (auto Enum = dyn_cast<ast::EnumDecl>(T))
      return Enum->getValue(Ident.getPart(1));
  }

  llvm_unreachable("invalid qualified expr");
  return nullptr;
}

Value *IRBuilder::visit(ast::IfExpr &If) {
  GetVal = true;
  auto Cond = If.getExpr()->accept(*this);
  GetVal = false;

  if (!Cond)
    Diagnostic(Module->getSourceFileName())
        .semanticError(If.getPosition(), "empty if condition");

  Cond = cmpWithTrue(Cond);

  auto Fn = Builder.GetInsertBlock()->getParent();
  auto ThenBB = BasicBlock::Create(Context, "then", Fn);
  auto MergeBB = BasicBlock::Create(Context, "ifcont");
  auto ElseBB =
      If.getElseBranch() ? BasicBlock::Create(Context, "else") : MergeBB;

  Builder.CreateCondBr(Cond, ThenBB, ElseBB);
  Builder.SetInsertPoint(ThenBB);

  if (!If.getBlock())
    Diagnostic(Module->getSourceFileName())
        .semanticError(If.getPosition(), "empty if block");
  auto Then = If.getBlock()->accept(*this);

  Builder.CreateBr(MergeBB);
  ThenBB = Builder.GetInsertBlock();

  Value *Else = nullptr;
  if (auto ElseBranch = If.getElseBranch()) {
    Fn->getBasicBlockList().push_back(ElseBB);
    Builder.SetInsertPoint(ElseBB);

    Else = Builder.CreateICmp(
        llvm::CmpInst::ICMP_NE, Cond,
        ConstantInt::get(Type::getInt1Ty(Context), 1, false), "ifcond");
    if (auto ElseBlock = ElseBranch->getBlock())
      ElseBlock->accept(*this);
    else
      Diagnostic(Module->getSourceFileName())
          .semanticError(If.getPosition(), "empty else block");

    Builder.CreateBr(MergeBB);
    ElseBB = Builder.GetInsertBlock();
  }

  Fn->getBasicBlockList().push_back(MergeBB);
  Builder.SetInsertPoint(MergeBB);

  PHINode *PN = Builder.CreatePHI(Type::getInt1Ty(Context), 2, "iftmp");

  PN->addIncoming(Cond, ThenBB);
  if (Else)
    PN->addIncoming(Else, ElseBB);

  return PN;
}

Value *IRBuilder::visit(ast::ForExpr &For) {
  auto Iter = static_cast<ast::LiteralExpr *>(For.getIter());

  Value *StartVal = nullptr;
  Value *EndVal = nullptr;

  if (auto Literal = dyn_cast<ast::LiteralExpr>(For.getRange())) {
    if (auto Ptr = dyn_cast<PointerType>(Literal->accept(*this)->getType()))
      if (auto Array = dyn_cast<ArrayType>(Ptr->getPointerElementType())) {
        StartVal = ConstantInt::get(Type::getInt32Ty(Context), 0);
        EndVal = ConstantInt::get(Type::getInt32Ty(Context),
                                  Array->getNumElements());
      }
  } else if (auto Range = dyn_cast<ast::RangeExpr>(For.getRange())) {
    StartVal = Range->getBeginValue()->accept(*this);
    EndVal = Range->getEndValue()->accept(*this);
  }

  if (!StartVal || !EndVal) {
    Diagnostic(Module->getSourceFileName())
        .semanticError(For.getPosition(), "invalid range");
  }

  auto Fn = Builder.GetInsertBlock()->getParent();
  auto PreheaderBB = Builder.GetInsertBlock();
  auto LoopBB = BasicBlock::Create(Context, "for_loop", Fn);

  Builder.CreateBr(LoopBB);
  Builder.SetInsertPoint(LoopBB);

  auto ASTVar = new ast::VarDecl(Iter->getTokenInfo(), true);
  auto IRVar =
      Builder.CreatePHI(Type::getInt32Ty(Context), 2, ASTVar->getIdentifier());
  IRVar->addIncoming(StartVal, PreheaderBB);

  ASTVar->setIRValue(IRVar);
  CurrentScope->addElement(ASTVar);

  auto Body = For.getBlock()->accept(*this);
  auto StepVal = ConstantInt::get(Context, APInt(32, 1));
  auto NextVar = Builder.CreateAdd(IRVar, StepVal, "nextvar");

  BasicBlock *LoopEndBB = Builder.GetInsertBlock();
  BasicBlock *AfterBB = BasicBlock::Create(Context, "afterloop", Fn);

  Builder.CreateCondBr(Builder.CreateICmpSLT(NextVar, EndVal), LoopBB, AfterBB);
  Builder.SetInsertPoint(AfterBB);

  IRVar->addIncoming(NextVar, LoopEndBB);

  return Constant::getNullValue(Type::getInt32Ty(Context));
}

Value *IRBuilder::visit(ast::WhileExpr &While) {
  auto Cond = While.getExpr()->accept(*this);

  if (!Cond)
    Diagnostic(Module->getSourceFileName())
        .semanticError(While.getPosition(), "invalid while expression");

  auto Fn = Builder.GetInsertBlock()->getParent();
  auto PreheaderBB = Builder.GetInsertBlock();
  auto LoopBB = BasicBlock::Create(Context, "while_loop", Fn);

  Builder.CreateBr(LoopBB);
  Builder.SetInsertPoint(LoopBB);

  auto IRVar = Builder.CreatePHI(Cond->getType(), 2, "phi");
  IRVar->addIncoming(Cond, PreheaderBB);

  While.getBlock()->accept(*this);

  BasicBlock *LoopEndBB = Builder.GetInsertBlock();
  BasicBlock *AfterBB = BasicBlock::Create(Context, "afterloop", Fn);

  Builder.CreateCondBr(While.getExpr()->accept(*this), LoopBB, AfterBB);
  Builder.SetInsertPoint(AfterBB);

  IRVar->addIncoming(Cond, LoopEndBB);

  return Constant::getNullValue(Type::getInt32Ty(Context));
}

#define ASSIGN(FN)                                                             \
  Builder.CreateStore(Builder.Create##FN(Builder.CreateLoad(LHS),              \
                                         RHS->getType()->isPointerTy()         \
                                             ? Builder.CreateLoad(RHS)         \
                                             : RHS),                           \
                      LHS);

Value *IRBuilder::visit(ast::AssignExpr &Assign) {
  GetVal = false;
  auto LHS = Assign.getLHS()->accept(*this),
       RHS = Assign.getRHS()->accept(*this);

  if (!LHS || !RHS)
    Diagnostic(Module->getSourceFileName())
        .semanticError(Assign.getPosition(), "invalid assign expression");

  switch (Assign.getOperator()) {
  case Token::Assign:
    return Builder.CreateStore(RHS, LHS);

  case Token::DivAssign:
    return ASSIGN(SDiv);

  case Token::MultAssign:
    return ASSIGN(Mul);

  case Token::PlusAssign:
    return ASSIGN(Add);

  case Token::MinusAssign:
    return ASSIGN(Sub);

  case Token::RShiftAssign:
    return ASSIGN(LShr);

  case Token::LShiftAssign:
    return ASSIGN(Shl);

  case Token::AndAssign:
    return ASSIGN(And);

  case Token::OrAssign:
    return ASSIGN(Or);

  default:
    llvm_unreachable("unsupported assign expression operator");
  }
}

Value *IRBuilder::visit(ast::StructInitExpr &Struct) {
  auto Ty = getTypeFromIdent(Struct.getIdentifier());
  auto TypeDef = static_cast<ast::TypeDef *>(Ty->getDecl());
  Struct.setType(static_cast<ast::StructDecl *>(TypeDef->getTypeDecl()));

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
      Diagnostic(Module->getSourceFileName())
          .semanticError(Array.getPosition(),
                         "array elements can't has different types");

    Values.push_back(static_cast<Constant *>(Elem));
  }

  return ConstantArray::get(Ty, Values);
}

} // namespace north::targets
