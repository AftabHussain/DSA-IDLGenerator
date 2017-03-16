//
// This file is distributed under the MIT License. See LICENSE for details.
//
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include "dsaGenerator/UndefinedFunctionsPass.h"

#include <set>

namespace dsa {

bool UndefinedFunctionsPass::runOnModule(llvm::Module &m) {
  std::set<std::string> listOfUndefinedFunctions;
  for (auto &F : m) {
    //F.dump();
    if(F.getName().find("llvm") == std::string::npos) {
      for(llvm::inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
        //I->dump();
        if(llvm::CallInst* ci = llvm::dyn_cast<llvm::CallInst>(&*I)) {
          llvm::Function* calledF = ci->getCalledFunction();
          if(calledF && !calledF->isDeclaration() && calledF->getName().find("llvm") == std::string::npos) {
            listOfUndefinedFunctions.insert(calledF->getName());
          }
        }
      }
    }
  }
  for(auto it = listOfUndefinedFunctions.begin(); it != listOfUndefinedFunctions.end();
       it++) {
    llvm::errs() << *it << "\n";
  }
  return false;
}

// Pass ID variable
char UndefinedFunctionsPass::ID = 0;
}
