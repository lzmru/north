//===------------------------------------------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "Grammar/Lexer.h"

using namespace north;

TEST_CASE( "001-Lexer", "[lexer]" ) {
  north::Lexer lexer("../test/tests/001.n");

  REQUIRE( lexer.getNextToken().Type == Token::Open );
  REQUIRE( lexer.getNextToken().Type == Token::Identifier );
  REQUIRE( lexer.getNextToken().Type == Token::Def );
  REQUIRE( lexer.getNextToken().Type == Token::Identifier );
  REQUIRE( lexer.getNextToken().Type == Token::LParen );
  REQUIRE( lexer.getNextToken().Type == Token::RParen );
  REQUIRE( lexer.getNextToken().Type == Token::Colon );
  lexer.incrementIndentLevel();
  REQUIRE( lexer.getNextToken().Type == Token::Identifier );
  REQUIRE( lexer.getNextToken().Type == Token::LParen );
  REQUIRE( lexer.getNextToken().Type == Token::RParen );
  REQUIRE( lexer.getNextToken().Type == Token::Int );
}