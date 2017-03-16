//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef UndefinedFunctionsPass_H
#define UndefinedFunctionsPass_H

#include "llvm/Pass.h"

namespace dsa {

class UndefinedFunctionsPass: public llvm::ModulePass {
public:
  static char ID;  //Pass identification, replacement for typeid
  UndefinedFunctionsPass() : llvm::ModulePass(ID) {}
  virtual bool runOnModule(llvm::Module &m);
};
  
}
#endif //UndefinedFunctionsPass_H
