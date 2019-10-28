//===--- CLI.cpp — Command line interface implementation --------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CLI.h"

#include <llvm/Support/raw_ostream.h>

namespace north {

Command CLI::getCommand() {
  if (strcmp(Args[1], "help") == 0)
    error();

  if (strcmp(Args[1], "build") == 0)
    return Command::Build;
  if (strcmp(Args[1], "dump-ast") == 0)
    return Command::DumpAST;
  if (strcmp(Args[1], "emit-ir") == 0)
    return Command::EmitIR;

  error();
}

BuildCommand CLI::getBuildFlags() {
  if (Count < 3 || strcmp(Args[2], "help") == 0) {
    printHelp(Command::Build);
    exit(0);
  }

  BuildCommand Command;
  Command.Input = Args[2];

  uint8_t Current = 3;

  while (Current < Count) {
    if (strncmp(Args[Current], "--target", 8) == 0) {
      if (strncmp((Args[Current] + 8), "=llvm", 5) == 0)
        Command.Target = CompilationTarget::LLVM;
      else if (strncmp((Args[Current] + 8), "=c", 2) == 0)
        Command.Target = CompilationTarget::C;
      else
        error();
    }
    
    if (strncmp(Args[Current], "--release", 8) == 0)
      Command.Build = BuildType::Release;
    
    if (strncmp(Args[Current], "-o", 2) == 0 || strncmp(Args[Current], "--output", 8) == 0)
      Command.Output = Args[++Current];

    ++Current;
  }

  return Command;
}

DumpASTCommand CLI::getDumpASTFlags() {
  DumpASTCommand Command;
  Command.Input = Args[2];
  return Command;
}

EmitIRCommand CLI::getEmitIRFlags() {
  EmitIRCommand Command;
  Command.Input = Args[2];
  return Command;
}

void CLI::printHelp(Command HelpFor) {
  switch (HelpFor) {
  case Command::Help:
    llvm::outs() << R"(
Usage: northc <command> file [options]
COMMAND:
  build
  dump-ast
  emit-ir
  help
)";
    break;

  case Command::Build:
    llvm::outs() << R"(
Usage: northc build file [options]
OPTIONS:
  --target    — compilation target
    =llvm
    =c
  --release   - release build
)";
    break;
    
  default:
    break;
  }
}

void CLI::error() {
  printHelp();
  std::exit(0);
}

} // namespace north
