//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef DSAGENERATOR_H
#define DSAGENERATOR_H

#include "dsa/DSGraph.h"
#include "dsa/DSNode.h"
#include "dsa/DataStructure.h"
#include "llvm/Pass.h"

namespace dsa {

using namespace llvm;

class DSAGenerator : public llvm::ModulePass {
private:
  BUDataStructures *BU;

public:
  static char ID; // Pass identification, replacement for typeid
  std::string passName;

  DSAGenerator() : llvm::ModulePass(ID), BU(NULL), passName("DSAGenerator") {}

  virtual const char *getPassName() const { return passName.c_str(); }

  virtual bool runOnModule(llvm::Module &m);

  virtual void getAnalysisUsage(AnalysisUsage &AU) const {
    AU.addRequired<BUDataStructures>();
    AU.setPreservesAll();
  }
};
}

#endif // DSAGENERATOR_H
