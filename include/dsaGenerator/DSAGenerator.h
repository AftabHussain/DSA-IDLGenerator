//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef DSAGENERATOR_H
#define DSAGENERATOR_H

#include "llvm/Pass.h"
#include "dsa/DataStructure.h"
#include "dsa/DSGraph.h"
#include "dsa/DSNode.h"

namespace dsa {

using namespace llvm;
  
class DSAGenerator: public llvm::ModulePass {
private:
BUDataStructures* BU;

public:
  static char ID; // Pass identification, replacement for typeid
  
  DSAGenerator() : llvm::ModulePass(ID) {}
  
  virtual bool runOnModule(llvm::Module& m);
  
  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BUDataStructures>();
    AU.setPreservesAll();
  }
};

}

#endif //DSAGENERATOR_H

