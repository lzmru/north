//===--- Opt.h — Configure LLVM optimizations -------------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef NORTHC_OPT_H
#define NORTHC_OPT_H

#include "Commands.h"
#include "Type/Module.h"

#include <llvm/Target/TargetMachine.h>

namespace north {

void configureOpimizations(llvm::TargetMachine *TM,
                           north::type::Module *Module,
                           const north::BuildCommand &Command,
                           llvm::raw_fd_ostream &dest);

} // namespace north 

#endif // NORTHC_OPT_H
