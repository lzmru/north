//===--- CLI.h — Command line interface -------------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTHC_CLI_H
#define NORTHC_CLI_H

#include "Commands.h"

namespace north {

class CLI final {
  int Count;
  const char **Args;

public:
  explicit CLI(int argc, const char *argv[])
    : Count(argc), Args(argv) { if (argc < 2) error(); }

  Command getCommand();
  
  BuildCommand getBuildFlags();
  DumpASTCommand getDumpASTFlags();
  EmitIRCommand getEmitIRFlags();

  void printHelp(Command For = Command::Help);
  void error();
};

} // namespace north

#endif // NORTHC_CLI_H
