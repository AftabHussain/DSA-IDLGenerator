//
// This file is distributed under the MIT License. See LICENSE for details.
//
#ifndef DSAGENERATOR_H
#define DSAGENERATOR_H

#include "dsa/DSGraph.h"
#include "dsa/DSNode.h"
#include "dsa/DataStructure.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/Pass.h"

namespace dsa {

class DSAGenerator : public llvm::ModulePass {
private:
  llvm::BUDataStructures *BU;
  std::string passName;
  std::string functionsList;

public:
  static char ID; // Pass identification, replacement for typeid

  DSAGenerator(std::string functionsList) : llvm::ModulePass(ID), BU(NULL), passName("DSAGenerator"), functionsList(functionsList) {}

  virtual const char *getPassName() const { return passName.c_str(); }
  std::string getFunctionsList() {return functionsList;}

  virtual bool runOnModule(llvm::Module &m);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const {
    llvm::errs()<<"[dsagen.h] Adding required passes of dsaGenerator: BUDataStructures\n";
    AU.addRequired<llvm::BUDataStructures>();
    AU.setPreservesAll();
  }
};
}

#endif // DSAGENERATOR_H
