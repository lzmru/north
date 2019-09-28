#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "Grammar/Lexer.h"

using namespace north;

TEST_CASE( "001-Lexer", "[lexer]" ) {
  auto MemBuff = llvm::MemoryBuffer::getFile("../../test/tests/001.n");
  llvm::SourceMgr SourceManager;
  SourceManager.AddNewSourceBuffer(std::move(*MemBuff), llvm::SMLoc());
  north::Lexer Lexer(SourceManager);

  REQUIRE( Lexer.getFlagState( Lexer:: LexerFlag::IndentationSensitive) == false);

  REQUIRE( Lexer.getNextToken().Type == Token::Open );
  REQUIRE( Lexer.getNextToken().Type == Token::Identifier );
  REQUIRE( Lexer.getNextToken().Type == Token::Def );
  REQUIRE( Lexer.getNextToken().Type == Token::Identifier );
  REQUIRE( Lexer.getNextToken().Type == Token::LParen );
  REQUIRE( Lexer.getNextToken().Type == Token::RParen );
  REQUIRE( Lexer.getNextToken().Type == Token::Colon );

  Lexer.incrementIndentLevel();
  REQUIRE( Lexer.getIndentLevel() == 1 );

  Lexer.turnFlag( Lexer::IndentationSensitive, true);
  REQUIRE( Lexer.getFlagState( Lexer:: LexerFlag::IndentationSensitive) == true);

  REQUIRE( Lexer.getNextToken().Type == Token::Indent );
  REQUIRE( Lexer.getNextToken().Type == Token::Identifier );
  REQUIRE( Lexer.getNextToken().Type == Token::LParen );
  REQUIRE( Lexer.getNextToken().Type == Token::RParen );
  REQUIRE( Lexer.getNextToken().Type == Token::Indent );
  REQUIRE( Lexer.getNextToken().Type == Token::Return );
  REQUIRE( Lexer.getNextToken().Type == Token::Int );
  REQUIRE( Lexer.getNextToken().Type == Token::Dedent );

  Lexer.decrementIndentLevel();
  REQUIRE( Lexer.getIndentLevel() == 0 );

  Lexer.turnFlag( Lexer::IndentationSensitive, false);
  REQUIRE( Lexer.getFlagState( Lexer:: LexerFlag::IndentationSensitive) == false);

  REQUIRE( Lexer.getNextToken().Type == Token::Eof );
}