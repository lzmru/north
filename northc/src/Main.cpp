//===--- Main.cpp — Compiler driver -----------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CLI.h"
#include "Dumper.h"
#include "Opt.h"

#include "Grammar/Parser.h"
#include "Targets/CBuilder.h"
#include "Targets/IRBuilder.h"
#include "Utils/FileSystem.h"

#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/PassManager.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Format.h>

namespace north {

void applyVisitor(ast::Visitor &V, type::Module *M) {
  for (auto I = M->getAST()->begin(), E = M->getAST()->end(); I != E; ++I)
    I->accept(V);
}

type::Module *parseModule(llvm::StringRef Path) {
  auto SrcMgr = utils::openFile(Path);
  Lexer Lexer(*SrcMgr);

  auto Module = new type::Module(
      Path, targets::IRBuilder::getContext(), *SrcMgr);
  Parser(Lexer, Module).parse();

  return Module;
}

void build(const BuildCommand &Command) {
  auto *Module = parseModule(Command.Input);

  if (Command.Target == CompilationTarget::C) {
    targets::CBuilder CBuilder(Module);
    applyVisitor(CBuilder, Module);
  } else {
    targets::IRBuilder IR(Module);
    applyVisitor(IR, Module);

    verifyModule(*Module, &llvm::outs());

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    Module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    if (!Target) {
      llvm::errs() << Error;
      return;
    }

    // FIXME
    auto CPU = "generic", Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TM = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    Module->setDataLayout(TM->createDataLayout());

    std::string Filename = Module->getModuleIdentifier() + ".o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::F_None);

    if (EC) {
      llvm::errs() << "couldn't open file: " << EC.message();
      return;
    }

    configureOpimizations(TM, Module, Command, dest);

    dest.flush();

    // FIXME
    auto Output = Command.Output.empty()
                      ? llvm::StringRef(Module->getModuleIdentifier())
                      : Command.Output;
    llvm::outs() << Filename << '\n' << Output << '\n';
    auto Cmd = "gcc " + Filename + " -o " + Output;
    system(Cmd.str().c_str());
  }
}

// TODO: Emit IR after optimizations
void emitIR(const EmitIRCommand &Command) {
  auto *Module = parseModule(Command.Input);

  targets::IRBuilder IRBuilder(Module);
  applyVisitor(IRBuilder, Module);

  verifyModule(*Module, &llvm::outs());

  llvm::outs() << *Module;
}

void dumpAST(const DumpASTCommand &Command) {
  ast::Dumper Dumper;
  applyVisitor(Dumper, parseModule(Command.Input));
}

} // namespace north

int main(int argc, const char *argv[]) {
  north::CLI CLI(argc, argv);

  switch (CLI.getCommand()) {
  case north::Command::Build:
    build(CLI.getBuildFlags());
    break;

  case north::Command::EmitIR:
    emitIR(CLI.getEmitIRFlags());
    break;

  case north::Command::DumpAST:
    dumpAST(CLI.getDumpASTFlags());
    break;
    
  default:
    break;
  }

  return 0;
}
