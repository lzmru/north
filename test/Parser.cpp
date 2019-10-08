#include <catch2/catch.hpp>
#include <Targets/IRBuilder.h>

#include "Grammar/Lexer.h"
#include "Grammar/Parser.h"
#include "Type/Module.h"
#include "AST/AST.h"

using namespace north;
using namespace north::ast;

class ParserTester {
  using ASTType = llvm::simple_ilist<ast::Node>;
  ASTType *AST;

public:
  explicit ParserTester(llvm::StringRef Path) {
    auto MemBuff = llvm::MemoryBuffer::getFile(Path);
    llvm::SourceMgr SourceManager;
    SourceManager.AddNewSourceBuffer(std::move(*MemBuff), llvm::SMLoc());

    auto Module = new type::Module(Path, north::targets::IRBuilder::getContext(), SourceManager);

    Lexer Lexer(SourceManager);
    north::Parser(Lexer, Module).parse();

    AST = Module->getAST();
  }

  void expectOpenStmt(llvm::StringRef ImportName) {
    REQUIRE( AST->front().getKind() == AST_OpenStmt );
    REQUIRE( ImportName == ((OpenStmt *)&AST->front())->getModuleName() );
    AST->pop_front();
  }

  void expectFuncDecl(llvm::StringRef Name,
                      std::function<void(FunctionDecl*)> DeclVerifier = nullptr,
                      std::function<void(BlockStmt*)> BlockVerifier = nullptr) {
    REQUIRE( AST->front().getKind() == AST_FunctionDecl );
    auto Fn = ((FunctionDecl *)&AST->front());
    REQUIRE( Fn->getIdentifier() == Name );
    if (DeclVerifier) DeclVerifier(Fn);
    if (BlockVerifier) BlockVerifier(Fn->getBlockStmt());
  }

  void expectReturnStmt(Node *N, NodeKind ExprKind) {
    REQUIRE( N->getKind() == AST_ReturnStmt );
    auto Return = (ReturnStmt *)&N;
    REQUIRE( Return->getReturnExpr()->getKind() == ExprKind );
  }

  void expectCallExpr(Node *N, llvm::StringRef Name,
      std::function<void(llvm::ArrayRef<CallExpr::Argument*>)> ArgsVerifier = nullptr) {
    REQUIRE( N->getKind() == AST_CallExpr );
    auto Callee = (CallExpr *)&N;
    if (ArgsVerifier) ArgsVerifier(Callee->getArgumentList());
  }
};

TEST_CASE( "001-Parser", "[parser]" ) {
  ParserTester Parser("../../test/tests/001.n");

  Parser.expectOpenStmt("Test");

  Parser.expectFuncDecl("printf", [] (FunctionDecl *Fn) {
    REQUIRE( Fn->getArg(0)->getIdentifier() == "_" );
    REQUIRE( Fn->isVarArg() );
  });

  Parser.expectFuncDecl("mult",
      [] (FunctionDecl *Fn) {
    REQUIRE( Fn->getGenericsList()[0].Name == "T" );
    REQUIRE( Fn->getArg(0)->getIdentifier() == "lhs" );
    REQUIRE( Fn->getArg(0)->getNamedArg() == "_" );
    REQUIRE( !Fn->isVarArg() );
    REQUIRE( Fn->getArg(1)->getIdentifier() == "rhs" );
    REQUIRE( Fn->getArg(1)->getNamedArg() == "rhs" );
    REQUIRE( Fn->getArg(1)->getType()->getIdentifier() == "T" );
    REQUIRE( Fn->getTypeDecl()->getIdentifier() == "T" );
  }, [&] (BlockStmt *Block) {
    Parser.expectReturnStmt(&Block->getBody()->front(), AST_BinaryExpr);
  });

  Parser.expectFuncDecl("main", nullptr, [&] (BlockStmt *Block) {
    Parser.expectCallExpr(&Block->getBody()->front(), "printf", [](llvm::ArrayRef<CallExpr::Argument*> Args) {
      REQUIRE( Args[0]->ArgName == "" );
      REQUIRE( Args[1]->ArgName == "random_vararg_label" );
      REQUIRE( Args[2]->ArgName == "" );
    });
  });

}