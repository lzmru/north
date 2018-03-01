//===--- Main.cpp - Compiler driver -----------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "AST/Dumper.h"
#include "Grammar/Parser.h"
#include "IR/IRBuilder.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

#include <clang/Basic/FileManager.h>
#include <llvm/Support/CommandLine.h>

#include <llvm/Support/Casting.h>

using namespace llvm;
using namespace llvm::sys;

static cl::opt<std::string>
    FileName(cl::Positional, cl::desc("<Path to sources>"), cl::Required);
static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Specify output filename"),
                                           cl::value_desc("filename"));
static cl::opt<bool> ASTDump("ast-dump", cl::desc("Dump AST"));

int main(int argc, const char *argv[]) {

  cl::ParseCommandLineOptions(argc, argv, "The North language compiler\n");

  north::Parser parser(FileName.c_str());
  auto Module = parser.parse();

  if (ASTDump.getValue()) {
    north::ast::Dumper D;
    for (auto I = Module->getAST()->begin(), E = Module->getAST()->end();
         I != E; ++I) {
      I->accept(D);
    }
  } else {

    north::ir::IRBuilder B(Module);
    for (auto I = Module->getAST()->begin(), E = Module->getAST()->end();
         I != E; ++I) {
      I->accept(B);
    }

    llvm::outs() << *Module;

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto TargetTriple = sys::getDefaultTargetTriple();
    Module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
      errs() << Error;
      return 1;
    }

    auto CPU = "generic", Features = "";

    TargetOptions opt;
    auto RM = Optional<Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    Module->setDataLayout(TheTargetMachine->createDataLayout());

    std::string Filename = Module->getModuleIdentifier() + ".o";
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::F_None);

    if (EC) {
      errs() << "Couldn't open file: " << EC.message();
      return 1;
    }

    legacy::PassManager pass;
    auto FileType = TargetMachine::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
      errs() << "TheTargetMachine can't emit a file of this type";
      return 1;
    }

    pass.run(*Module);
    dest.flush();

    system(("gcc " + Filename + " -o " +
            ((OutputFilename.getValue() == "") ? Module->getModuleIdentifier()
                                               : OutputFilename.getValue()))
               .c_str());
  }

  return 0;
}
