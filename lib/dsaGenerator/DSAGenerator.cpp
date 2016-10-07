//
// This file is distributed under the MIT License. See LICENSE for details.
//
#include "dsaGenerator/DSAGenerator.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/LLVMContext.h"

#include <fstream>

namespace dsa {

using namespace llvm;

void printOffsets(DSNode* node, std::string indentation, std::ofstream *file, std::vector<DSNode*> *visitedNodes) {
          *file << indentation << "Write\n";
          for(DSNode::const_offset_iterator it=node->write_offset_begin(); it!=node->write_offset_end(); it++) {
            *file << indentation << *it << " ";
          }
          *file << "\n";
          *file << indentation << "Read\n";
          for(DSNode::const_offset_iterator it=node->read_offset_begin(); it!=node->read_offset_end(); it++) {
            *file << indentation << *it << " ";
          }
          *file << "\n";
          visitedNodes->push_back(node);
          for(DSNode::const_edge_iterator ei=node->edge_begin(); ei!=node->edge_end(); ei++) {
            if(std::find(visitedNodes->begin(), visitedNodes->end(), (*ei).second.getNode()) != visitedNodes->end()) continue;
            *file << indentation << "{offset: " << (*ei).first << "\n";
            printOffsets((*ei).second.getNode(), indentation, file, visitedNodes);
            *file << "}\n\n";
          }
}

bool DSAGenerator::runOnModule(Module& m) {
  BU = &getAnalysis<BUDataStructures>();
  for (auto& F : m) {
    if (F.getName().find("llvm") == std::string::npos) {
      errs() << "Function " << F.getName() << "\n";
      if(F.isDeclaration()) {
        errs() << "just function declaration, skipping \n";
        continue;
      }
      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode*> argumentNodes;
      int paramNumber=0;
      std::ofstream file(F.getName());
      for (Function::arg_iterator A=F.arg_begin(); A!=F.arg_end(); A++, paramNumber++) {
        file << "parameter: " << paramNumber << "\n";
	Value* val = dyn_cast<Value>(&*A);
        if(!val || !val->getType()->isPointerTy()) {
	  argumentNodes.push_back(NULL);
	} else {
          DSNodeHandle &nodeHandle = graph->getNodeForValue(val);
          DSNode *node = nodeHandle.getNode();
          argumentNodes.push_back(node);
          file << "Write\n";
          for(DSNode::const_offset_iterator it=node->write_offset_begin(); it!=node->write_offset_end(); it++) {
            file << *it << " ";
          }
          file << "\n";
          file << "Read\n";
          for(DSNode::const_offset_iterator it=node->read_offset_begin(); it!=node->read_offset_end(); it++) {
            file << *it << " ";
          }
          file << "\n";
          std::string indentation("");
          std::vector<DSNode*> visitedNodes;
          visitedNodes.push_back(node);
          for(DSNode::const_edge_iterator ei=node->edge_begin(); ei!=node->edge_end(); ei++) {
            if(std::find(visitedNodes.begin(), visitedNodes.end(), (*ei).second.getNode()) != visitedNodes.end()) continue;
            file << indentation << "{\noffset: " << (*ei).first << "\n";
            printOffsets((*ei).second.getNode(), indentation, &file, &visitedNodes);
            file << "}\n\n";
          }
	}
        file << "\n";
      }
    }
  }
 return false;  
}
// Pass ID variable
char DSAGenerator::ID = 0;
}
