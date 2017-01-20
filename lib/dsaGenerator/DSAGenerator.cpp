//
// This file is distributed under the MIT License. See LICENSE for details.
//
#include "dsaGenerator/DSAGenerator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Support/Debug.h"

#include <fstream>

namespace dsa {

using namespace llvm;

typedef std::map<unsigned, std::pair<std::string, DIType *>> offsetNames;
DITypeIdentifierMap TypeIdentifierMap;
void offsetPrinter(const DSNode &node, std::ofstream &file, StringRef op,
                   offsetNames &of, std::string);
void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off,
                 std::string baseName, std::string indent, Argument& arg);

/// Prints it in console. Solely for debugging purposes
void dumpOffsetNames(offsetNames &of) {
  errs() << "-------------------------------------------\n";
  for (auto off : of) {
    errs() << "\toffset : " << off.first << "\t";
    errs() << " | name : " << std::get<0>(off.second) << "\n";
  }
  errs() << "-------------------------------------------\n";
}

void printOffsets(DSNode *node, std::string indentation, std::ofstream *file,
                  std::vector<DSNode *> *visitedNodes, offsetNames &of2, Argument& arg) {

  offsetPrinter(*node, *file, "read", of2, indentation);
  offsetPrinter(*node, *file, "write", of2, indentation);

  visitedNodes->push_back(node);
  for (DSNode::const_edge_iterator ei = node->edge_begin();
       ei != node->edge_end(); ei++) {
    if (std::find(visitedNodes->begin(), visitedNodes->end(),
                  (*ei).second.getNode()) != visitedNodes->end())
      continue;
    offsetNames of3;
    std::string ind2 = indentation;
    ind2.append("\t");

    unsigned offset = (*ei).first;
    std::string offset_name;
    offsetNames ofInner;

    // FIXME: exception handling may be removed once dsa gives back the correct
    // offsets.
    // Have this now to avoid segfault
    try {
      offset_name = of2.at(offset).first;
      *file << "\n" << ind2 << "{\n";
      *file << ind2 << "offset: " << offset << " | name: " << offset_name
            << "\n";
    } catch (std::out_of_range &e) {
      errs() << "OUT of range exception " << e.what() << "\n";
      continue;
    }
    DIType *Ty = of2.at(offset).second;
    //errs() << Ty->getTag() << "\n";
    // Previous offset is zero. No continuation
    getAllNames(Ty, ofInner, 0, offset_name, " ", arg);
    dumpOffsetNames(ofInner);

    printOffsets((*ei).second.getNode(), ind2, file, visitedNodes, ofInner, arg);

    *file << ind2 << "}\n\n";
  }
}

void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off,
                 std::string baseName, std::string indent, Argument& arg) {
  // Handle Pointer type
  if (Ty->getTag() == dwarf::DW_TAG_pointer_type ||
       Ty->getTag() == dwarf::DW_TAG_member) {
    DIType *baseTy =
        dyn_cast<DIDerivedType>(Ty)->getBaseType().resolve(TypeIdentifierMap);
    if (!baseTy) {
      errs() << "Type : NULL - Nothing more to do\n";
      return;
    }
    
    //Skip all the DINodes with DW_TAG_typedef tag
    while (baseTy->getTag() == dwarf::DW_TAG_typedef) {
      baseTy = dyn_cast<DIDerivedType>(baseTy)->getBaseType().resolve(TypeIdentifierMap);
    }
    
    // If that pointer is a struct 
    if (baseTy->getTag() == dwarf::DW_TAG_structure_type) {
      DICompositeType *compType = dyn_cast<DICompositeType>(baseTy);

      // Go thro struct elements and print them all
      for (DINode *Op : compType->getElements()) {
        DIDerivedType *der = dyn_cast<DIDerivedType>(Op);
        unsigned offset = der->getOffsetInBits() >> 3;

        /// XXX: crude assumption that we want to peek only into those members
        /// whose sizes are greater than 8 bytes
        if (((der->getSizeInBits() >> 3) > 8) &&
            der->getBaseType().resolve(TypeIdentifierMap)->getTag()) {
          std::string ind2 = indent;
          ind2.append("   ");
          errs() << der->getName().str();
          getAllNames(dyn_cast<DIType>(der), of, prev_off + offset,
                      der->getName().str(), ind2, arg);
        }
        std::string new_name(baseName);
        new_name.append(".");
        new_name.append(der->getName().str());
        //errs() << "--------------- " << der->getName().str() << "\n";
        of[offset + prev_off] = std::pair<std::string, DIType *>(
            new_name, der->getBaseType().resolve(TypeIdentifierMap));
      }
    } else if (DIBasicType *bas = dyn_cast<DIBasicType>(baseTy)) {
      //if type tag for the parameter is of pointer_type and DI type is DIBasicType 
      //then treat it as a pointer of native type
      of[0] = std::pair<std::string, DIType *>(
            arg.getName().str(), bas);
    }
  }
}

offsetNames getArgFieldNames(Function &F, unsigned argNumber, Argument& arg) {
  offsetNames offNames;
  assert((argNumber == 0) &&
         "Request for return type information. Not supported");
  if (argNumber > F.arg_size()) {
    errs() << "### WARN : requested data for non-existent element\n";
    return offNames;
  }
  errs() << "## Function ## : " << F.getName().str()
         << " | argsize: " << F.arg_size() << " :: Requested " << argNumber
         << "\n";
  SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
  F.getAllMetadata(MDs);
  for (auto &MD : MDs) {
    if (MDNode *N = MD.second) {
      if (auto *subRoutine = dyn_cast<DISubprogram>(N)->getType()) {
        if (!subRoutine->getTypeArray()[0]) {
          errs() << "return type \"void\" for Function : " << F.getName().str()
                 << "\n";
        }
        if (DITypeRef ArgTypeRef = subRoutine->getTypeArray()[argNumber]) {
          // Resolve the type
          DIType *Ty = ArgTypeRef.resolve(TypeIdentifierMap);
          // Handle Pointer type
          getAllNames(Ty, offNames, 0, "", "  ", arg);
        }
      }
    }
  }
  return offNames;
}

void offsetPrinter(const DSNode &node, std::ofstream &file, StringRef op,
                   offsetNames &of, std::string indent) {
  DSNode::const_offset_iterator ii, ei;
  if (op.str() == "read") {
    file << "\n" << indent << "Read: \n";
    ii = node.read_offset_begin();
    ei = node.read_offset_end();
  } else if (op.str() == "write") {
    file << "\n" << indent << "Write: \n";
    ii = node.write_offset_begin();
    ei = node.write_offset_end();
  }

  for (; ii != ei; ii++) {
    unsigned offset = *ii;
    std::string Name("????");
    if (of.find(offset) != of.end()) {
      Name = of.at(offset).first;
    }
    file << indent << " offset: " << *ii << "\t\t" << Name << "\n";
  }
}

bool DSAGenerator::runOnModule(Module &m) {
  BU = &getAnalysis<BUDataStructures>();
  if (NamedMDNode *CU_Nodes = m.getNamedMetadata("llvm.dbg.cu")) {
    TypeIdentifierMap = generateDITypeIdentifierMap(CU_Nodes);
  }

  for (auto &F : m) {
    if (F.getName().find("llvm") == std::string::npos) {
      if (F.isDeclaration()) {
        errs() << F.getName() << " just function declaration, skipping \n";
        continue;
      }

      DSGraph *graph = BU->getDSGraph(F);
      std::vector<DSNode *> argumentNodes;
      std::ofstream file(F.getName());

      //errs() << "has metadata: " << F.hasMetadata() << "\n";
      for (auto &arg : F.args()) {
        if (arg.hasName()) {
          file << arg.getArgNo() << " = arg -->" << arg.getName().str() << "\n";
        }

        // XXX: What about non-pointer variables ??
        if (arg.getType()->isPointerTy()) {
          offsetNames of = getArgFieldNames(F, arg.getArgNo() + 1, arg);
          dumpOffsetNames(of);
          DSNodeHandle &nodeHandle = graph->getNodeForValue(&arg);
          DSNode *node = nodeHandle.getNode();

          std::vector<DSNode *> visitedNodes;

          visitedNodes.push_back(node);
          printOffsets(node, "", &file, &visitedNodes, of, arg);
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
