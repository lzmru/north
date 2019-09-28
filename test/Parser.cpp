#include <catch2/catch.hpp>

#include "Grammar/Lexer.h"
#include "Grammar/Parser.h"
#include "Type/Module.h"
#include "AST/AST.h"

using namespace north;
using namespace north::ast;

#define TEST_NODE(TYPE, EXPECTED, RESULT) \
  REQUIRE( AST->front().getKind() == NodeKind::AST_##TYPE ); \
  auto TYPE##_##EXPECTED##_##RESULT = static_cast<TYPE *>(&AST->front())->RESULT(); \
  REQUIRE( #EXPECTED == TYPE##_##EXPECTED##_##RESULT ); \
  AST->pop_front();

#define TEST_FUNCTION(NAME, TYPE, BODY_TEST) \
  REQUIRE( AST->front().getKind() == NodeKind::AST_FunctionDecl ); \
  auto NAME##TYPE##_fndecl = static_cast<FunctionDecl *>(&AST->front()); \
  REQUIRE( #NAME == NAME##TYPE##_fndecl->getIdentifier() ); \
  REQUIRE( TYPE == NAME##TYPE##_fndecl->getTypeDecl() ); \
  REQUIRE( nullptr != NAME##TYPE##_fndecl->getBlockStmt() ); \
  auto NAME##TYPE##_ast_buffer = AST; \
  AST = NAME##TYPE##_fndecl->getBlockStmt()->getBody(); \
  BODY_TEST(NAME##TYPE##_fndecl->getBlockStmt()); \
  AST = NAME##TYPE##_ast_buffer; \
  AST->pop_front();

#define CHECK_CALL(IDENTIFIER, COUNT_OF_ARGS) \
  REQUIRE( AST->front().getKind() == NodeKind::AST_CallExpr ); \
  auto IDENTIFIER##EXPECTED##_callExpr = static_cast<CallExpr *>(&AST->front()); \
  auto IDENTIFIER##EXPECTED##_identifier = IDENTIFIER##EXPECTED##_callExpr->getIdentifier(0); \
  REQUIRE( #IDENTIFIER == IDENTIFIER##EXPECTED##_identifier ); \
  REQUIRE( COUNT_OF_ARGS ==  IDENTIFIER##EXPECTED##_callExpr->numberOfArgs() ); \
  AST->pop_front();

TEST_CASE( "001-Parser", "[parser]" ) {
  llvm::StringRef Path = "../../test/tests/001.n";

  auto MemBuff = llvm::MemoryBuffer::getFile(Path);
  llvm::SourceMgr SourceManager;
  SourceManager.AddNewSourceBuffer(std::move(*MemBuff), llvm::SMLoc());

  auto Module = new type::Module(Path, north::targets::IRBuilder::getContext(), SourceManager);

  Lexer Lexer(SourceManager);
  north::Parser(Lexer, Module).parse();

  auto AST = Module->getAST();

  TEST_NODE(OpenStmt, T, getModuleName)
  TEST_FUNCTION(t, nullptr, [&](BlockStmt *Block) {
    CHECK_CALL(test, 0)
    REQUIRE( AST->front().getKind() == AST_ReturnStmt );
  })

}