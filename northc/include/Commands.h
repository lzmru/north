//===--- Commands.h ---------------------------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTHC_COMMANDS_H
#define NORTHC_COMMANDS_H

#include <llvm/ADT/StringRef.h>

namespace north {

enum class Command {
  Help,
  Build,
  DumpAST,
  EmitIR,
};

enum class BuildType { Debug, Release };
enum class CompilationTarget { LLVM, C };

struct BuildCommand {
  BuildType Build = BuildType::Debug;
  CompilationTarget Target = CompilationTarget::LLVM;
  llvm::StringRef Input;
  llvm::StringRef Output;
};

struct DumpASTCommand {
  llvm::StringRef Input;
};

struct EmitIRCommand {
  llvm::StringRef Input;
};

} // namespace north

#endif // NORTHC_COMMANDS_H
