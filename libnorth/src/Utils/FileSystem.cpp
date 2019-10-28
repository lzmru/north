//===--- Utils/FileSystem.cpp - File system utilities -----------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Utils/FileSystem.h"

namespace north::utils {
  
llvm::SourceMgr *openFile(llvm::StringRef Path) {
  auto MemBuff = llvm::MemoryBuffer::getFile(Path);
  if (auto Error = MemBuff.getError()) {
    llvm::errs() << Path << ": " << Error.message() << '\n';
    std::exit(0);
  }

  if (!MemBuff->get()->getBufferSize())
    std::exit(0);

  auto SourceManager = new llvm::SourceMgr();
  SourceManager->AddNewSourceBuffer(std::move(*MemBuff), llvm::SMLoc());

  SourceManager->setDiagHandler([](const llvm::SMDiagnostic &SMD, void *Context) {
    SMD.print("", llvm::errs());
    std::exit(0);
  });
  
  return SourceManager;
}

} // north::utils

