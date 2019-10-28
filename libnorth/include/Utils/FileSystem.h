//===--- Utils/FileSystem.h - File system utilities -------------*- C++ -*-===//
//
//                       The North Compiler Infrastructure
//
//                This file is distributed under the MIT License.
//                        See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LIBNORTH_UTILS_FILEMANAGER_H
#define LIBNORTH_UTILS_FILEMANAGER_H

#include <llvm/Support/SourceMgr.h>

namespace north::utils {
  
llvm::SourceMgr *openFile(llvm::StringRef Path);

} // north::utils

#endif // LIBNORTH_UTILS_FILEMANAGER_H
