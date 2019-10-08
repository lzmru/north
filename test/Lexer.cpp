#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <utility>

#include "Grammar/Lexer.h"

using namespace north;

class LexerTester {
  std::unique_ptr<Lexer> Lex;

public:
  explicit LexerTester(llvm::StringRef Filename) {
    auto MemBuff = llvm::MemoryBuffer::getFile(Filename);
    llvm::SourceMgr SourceManager;
    SourceManager.AddNewSourceBuffer(std::move(*MemBuff), llvm::SMLoc());
    Lex = std::make_unique<Lexer>(SourceManager);

    checkFlag(Lexer::LexerFlag::IndentationSensitive, false);
  }

  ~LexerTester() {
    REQUIRE( Lex->getNextToken().Type == Token::Eof );
  }

  void expectIndent() {
    for (int I = Lex->getIndentLevel(); I > 0; --I)
      REQUIRE( Lex->getNextToken().Type == Token::Indent );
  }

  void checkFlag(Lexer::LexerFlag Flag, bool state) {
    REQUIRE( Lex->getFlagState(Flag) == state );
  }

  void expectOpenStmt(llvm::StringRef Module) {
    REQUIRE( Lex->getNextToken().Type == Token::Open );
    auto Ident = Lex->getNextToken();
    REQUIRE( Ident.Type == Token::Identifier );
    REQUIRE( Ident.toString() == Module );
  }

  void expectFunctionDef(llvm::StringRef Name,
                         llvm::StringRef Generics,
                         llvm::StringRef Args,
                         llvm::StringRef ReturnType,
                         std::function<void(LexerTester*)> Block) {
    REQUIRE( Lex->getNextToken().Type == Token::Def );
    REQUIRE( Lex->getNextToken().Type == Token::Identifier );

    if (!Generics.empty()) {
      REQUIRE( Lex->getNextToken().Type == Token::LBracket );

      TokenInfo Tk = Lex->getNextToken();
      auto* Start = Tk.Pos.Offset;
      while (Tk.Type != Token::RBracket) {
        Tk = Lex->getNextToken();
        REQUIRE( Tk.Type != Token::Eof );
      }
      size_t End = (size_t)((Tk.Pos.Offset + Tk.Pos.Length) - Start - 1);

      REQUIRE( Generics == llvm::StringRef(Start, End) );
    }

    REQUIRE( Lex->getNextToken().Type == Token::LParen );

    if (!Args.empty()) {
      TokenInfo Tk = Lex->getNextToken();
      auto* Start = Tk.Pos.Offset;
      while (Tk.Type != Token::RParen) {
        Tk = Lex->getNextToken();
        REQUIRE( Tk.Type != Token::Eof );
      }
      size_t End = (size_t)((Tk.Pos.Offset + Tk.Pos.Length) - Start - 1);

      REQUIRE( Args == llvm::StringRef(Start, End) );
    } else {
      REQUIRE( Lex->getNextToken().Type == Token::RParen );
    }

    if (!ReturnType.empty()) {
      TokenInfo Tk = Lex->getNextToken();

      REQUIRE( Tk.Type == Token::RightArrow );
      Tk = Lex->getNextToken();
      auto* Start = Tk.Pos.Offset;
      while (Tk.Type != Token::Colon) {
        Tk = Lex->getNextToken();
        REQUIRE( Tk.Type != Token::Eof );
      }
      size_t End = (size_t)((Tk.Pos.Offset + Tk.Pos.Length) - Start - 1);

      REQUIRE( ReturnType == llvm::StringRef(Start, End) );
    }

    if (Block) {
      if (ReturnType.empty()) REQUIRE( Lex->getNextToken().Type == Token::Colon );
      Lex->incrementIndentLevel();
      Lex->turnFlag( Lexer::IndentationSensitive, true);
      Block(this);
      Lex->decrementIndentLevel();
      Lex->turnFlag( Lexer::IndentationSensitive, false);
    }
  }

  void expectReturnStmt(llvm::StringRef Value) {
    REQUIRE( Lex->getNextToken().Type == Token::Return );
    TokenInfo Tk = Lex->getNextToken();
    auto* Start = Tk.Pos.Offset;
    while (Tk.Type != Token::Dedent) {
      Tk = Lex->getNextToken();
      REQUIRE( Tk.Type != Token::Eof );
    }
    size_t End = (size_t)((Tk.Pos.Offset + Tk.Pos.Length) - Start - 1);
    if (*(Start+End) == '\n') --End;
    REQUIRE( llvm::StringRef(Start, End) == Value );
  }

  void expectCallExpr(llvm::StringRef Name, llvm::StringRef Args) {
    auto Ident = Lex->getNextToken();
    REQUIRE( Ident.Type == Token::Identifier );
    REQUIRE( Ident.toString() == Name );
    REQUIRE( Lex->getNextToken().Type == Token::LParen );

    if (!Args.empty()) {

      TokenInfo Tk = Lex->getNextToken();
      auto* Start = Tk.Pos.Offset;
      while (Tk.Type != Token::Dedent && Tk.Type != Token::Indent) {
        Tk = Lex->getNextToken();
        REQUIRE( Tk.Type != Token::Eof );
      }
      size_t End = (size_t)((Tk.Pos.Offset + Tk.Pos.Length) - Start - 2);
      REQUIRE( llvm::StringRef(Start, End) == Args );
    } else {
      REQUIRE( Lex->getNextToken().Type == Token::RParen );
    }

  }
};

TEST_CASE( "001-Lexer", "[lexer]" ) {
  LexerTester Lex("../../test/tests/001.n");

  Lex.expectOpenStmt("Test");
  Lex.expectFunctionDef("printf", {}, "_: *i8, ...", "",nullptr);

  Lex.expectFunctionDef("mult", "T", "_ lhs: T, rhs: T", "T", [](LexerTester *Lex) {
    Lex->expectIndent();
    Lex->expectReturnStmt("lhs * rhs");
  });

  Lex.expectFunctionDef("main", "" , "", "", [](LexerTester *Lex) {
    Lex->expectIndent();
    Lex->expectCallExpr("printf", "\"%s: %d\", random_vararg_label: \"mult() res:\", mult(5, rhs: 5)");
  });

}