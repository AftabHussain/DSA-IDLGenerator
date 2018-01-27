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
#include "llvm/IR/DebugInfo.h"
#include "llvm/Support/FileSystem.h"

#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iterator>
#include <algorithm>
#include <unordered_set>

namespace dsa {

	using namespace llvm;

	typedef std::map<unsigned, std::pair<std::string, DIType *>> offsetNames;
	DITypeIdentifierMap TypeIdentifierMap;
	void offsetPrinter(const DSNode &node, std::ofstream &file, StringRef op,
			offsetNames &of, std::string, std::string);
	void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off,
			std::string baseName, std::string indent, StringRef argName, std::string&);

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
			std::vector<DSNode *> *visitedNodes, offsetNames &of2, 
			Type* elementType, StringRef elementName, std::string& structName, std::string functionName) {
		if(structName != "") {
			
			//------UNEXAMINED (MAY BE USEFUL LATER)---------------//
			//arg.dump();
			//errs() << "\nis pointer: " << isa<PointerType>(arg.getType()) << "\n";
			//errs() << "\nis struct: " << dyn_cast<StructType>(dyn_cast<Pointe
			//rType>(arg.getType())->getElementType())->getName().str() << "\n";
			//if(dyn_cast<PointerType>(arg.getType())->getElementType()->getStructName() != NULL)
			//*file << indentation << "\nprojection <struct " << arg
			//.getType()
			//->getStructName()
			//     .str() << "> " << structName << " {";
			//else
			//------------------------------------------------------//
			
			Type* argType = elementType;

			while(!isa<StructType>(argType)) {
				argType = dyn_cast<PointerType>(argType)->getElementType();
			}

			*file << indentation << "\nprojection <struct " << dyn_cast<StructType>(argType)->getName().substr(7).str() << "> " << functionName << "." << elementName.str() << " {\n";
                        
			//*file << (node->isCollapsedNode() ? "collapsed\n" : "not collapsed\n");
			offsetPrinter(*node, *file, "read", of2, functionName, indentation);
			offsetPrinter(*node, *file, "write", of2, functionName, indentation);
			*file << indentation << "}\n\n";
		} 

		//else
		//*file << "\n" << indentation << "{\n";

		visitedNodes->push_back(node);
	
		for (DSNode::const_edge_iterator ei = node->edge_begin();ei != node->edge_end(); ei++) {
			
			if (std::find(visitedNodes->begin(), visitedNodes->end(),(*ei).second.getNode()) != visitedNodes->end())
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
				//*file << "\n" << ind2 << "{\n";
				//*file << ind2 << "offset: " << offset << " | name: " << offset_name
				//<< "\n";
				//*file << "collapsed: " << (*ei).second.getNode()->isCollapsedNode() << "\n"; 
				//*file << "forward: " << (*ei).second.getNode()->isForwarding() << "\n"; 
			} catch (std::out_of_range &e) {
				errs() << "OUT of range exception " << e.what() << "\n";
                                errs() << "offset: " << offset << "\n";
				continue;
			}
			
			DIType *Ty = of2.at(offset).second;
			//errs() << Ty->getTag() << "\n";
                        structName = "";
			// Previous offset is zero. No continuation
			getAllNames(Ty, ofInner, 0, offset_name, " ", elementName, structName);
			dumpOffsetNames(ofInner);
			//assuming that we will have to print this projection only once
			std::string emptyString("");
			printOffsets((*ei).second.getNode(), ind2, file, visitedNodes, ofInner, elementType, elementName, emptyString, emptyString);
		}
	}

	DIType* getLowestDINode(DIType* Ty) {
		if (Ty->getTag() == dwarf::DW_TAG_pointer_type ||
				Ty->getTag() == dwarf::DW_TAG_member || Ty->getTag() == dwarf::DW_TAG_typedef) {
			DIType *baseTy =
				dyn_cast<DIDerivedType>(Ty)->getBaseType().resolve(TypeIdentifierMap);
			if (!baseTy) {
				errs() << "Type : NULL - Nothing more to do\n";
				return NULL;
			}

			//Skip all the DINodes with DW_TAG_typedef tag
			while ((baseTy->getTag() == dwarf::DW_TAG_typedef || baseTy->getTag() == dwarf::DW_TAG_const_type 
						|| baseTy->getTag() == dwarf::DW_TAG_pointer_type)) {
				if (DITypeRef temp = dyn_cast<DIDerivedType>(baseTy)->getBaseType())
					baseTy = temp.resolve(TypeIdentifierMap);
				else
					break;
			}  
			return baseTy;
		}
		return Ty;
	}

	void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off,
			std::string baseName, std::string indent, StringRef argName, std::string& structName) {
		//if(prev_off >= 1024) return;
		// Handle Pointer type

		//TODO need to find out exactly why we need the type of the lowest node.
		DIType* baseTy = getLowestDINode(Ty);
		if (!baseTy)
			return;
		// If that pointer is a struct 
		if (baseTy->getTag() == dwarf::DW_TAG_structure_type) {
			//*file << "projection <struct " << arg->getType()->getStructName().str() << "" ;
			structName = baseTy->getName().str();
			DICompositeType *compType = dyn_cast<DICompositeType>(baseTy);

			// Go thro struct elements and print them all
			for (DINode *Op : compType->getElements()) {
				DIDerivedType *der = dyn_cast<DIDerivedType>(Op);
				unsigned offset = der->getOffsetInBits() >> 3;
				std::string new_name(baseName);
                                if (new_name != "") new_name.append(".");
				new_name.append(der->getName().str());
				of[offset + prev_off] = std::pair<std::string, DIType *>(
						new_name, der->getBaseType().resolve(TypeIdentifierMap));
				/// XXX: crude assumption that we want to peek only into those members
				/// whose sizes are greater than 8 bytes
				if (((der->getSizeInBits() >> 3) > 8) 
						&& der->getBaseType().resolve(TypeIdentifierMap)->getTag()) {
					std::string tempStructName("");
					getAllNames(dyn_cast<DIType>(der), of, prev_off + offset,
							new_name, indent, argName, tempStructName);
				}
				//errs() << "--------------- " << der->getName().str() << "\n";
			}
		} else if (DIBasicType *bas = dyn_cast<DIBasicType>(baseTy)) {
			structName = "";
			//if type tag for the parameter is of pointer_type and DI type is DIBasicType 
			//then treat it as a pointer of native type
			of[0] = std::pair<std::string, DIType *>(
					argName.str(), bas);
		} else {
			structName = "";
		}
	}

	//Gets the field names of a structure
	offsetNames getArgFieldNames(Function &F, unsigned argNumber, StringRef argName, std::string& structName) {

		errs() << "F.getName(){"<<F.getName()<<"}\n";
		//errs() << "number of arguments";
		offsetNames offNames;

		//it should be assert((argNumber != 0)....
		//assert((argNumber == 0) && "Request for return type information. Not supported");
	
		//didn't find any such case
		if (argNumber > F.arg_size()) {
			errs() << "### WARN : requested data for non-existent element\n";
			return offNames;
		}
		errs() << "## Function ## : " << F.getName().str()
		<< " | argsize: " << F.arg_size() << " :: Requested " << argNumber
		<< "\n";
		//GGL smallvector llvm
		SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
		F.getAllMetadata(MDs);
		for (auto &MD : MDs) {
			if (MDNode *N = MD.second) {
				if (auto *subRoutine = dyn_cast<DISubprogram>(N)->getType()) {
					if (!subRoutine->getTypeArray()[0]) {
						errs() << "return type \"void\" for Function : " << F.getName().str()<< "\n";
					}

					const auto &TypeRef = subRoutine->getTypeArray();

					/// XXX: When function arguments are coerced in IR, the corresponding
					/// debugInfo extracted for that function from the source code will
					/// not have the same number of arguments. Check the indexes to
					/// prevent array out of bounds exception (segfault)
					//did not encounter this case with dummy.c
					if (argNumber >= TypeRef.size()) {
						errs() << "TypeArray request out of bounds. Are parameters coerced??\n";
						goto done;
					}

					if (const auto &ArgTypeRef = TypeRef[argNumber]) {
						// Resolve the type
						DIType *Ty = ArgTypeRef.resolve(TypeIdentifierMap);
						// Handle Pointer type
						getAllNames(Ty, offNames, 0, "", "  ", argName, structName);
				}
				}
			}
		}	
		done:
		return offNames;
	}

	std::string getTypeNameFromDINode(DIType* dt) {
		while(dt->getName().str() == "") {
			dt = dyn_cast<DIDerivedType>(dt)->getBaseType().resolve(TypeIdentifierMap);
		}
		return dt->getName().str();
	}

	std::string getTypeName(DIType* dt, std::string function, std::string fieldName) {
		if (DIBasicType* bt = dyn_cast<DIBasicType>(dt)) {
			return bt->getName().str() + " " + fieldName;
		} else if (DICompositeType* ct = dyn_cast<DICompositeType>(dt)) {
			if (ct->getTag() == dwarf::DW_TAG_union_type) {
				return "union " + fieldName;
			} else {
			return "projection " + function + "." + ct->getName().str() + " *" + fieldName;
			}
		} else if (DISubroutineType* sr = dyn_cast<DISubroutineType>(dt)) {
		//if (!sr->getType()) {
		//return "rpc function " + "(*" + fieldName + ")()";  
		//} else {
			std::string name("rpc ");
			const DITypeRefArray &types = sr->getTypeArray();
			if (!types[0]) {
				//return type void
				name += "void (*" + fieldName + ") (";
			} else {
				DIType *ty = types[0].resolve(TypeIdentifierMap);
				//DIType *baseTy = getLowestDINode(ty);

				name += getTypeNameFromDINode(ty) + " (*" + fieldName + ") (";
			}
			unsigned int paramIndex = 1;
			while (paramIndex < types.size()) {
				DIType *ty = types[paramIndex].resolve(TypeIdentifierMap);
				ty->dump();
				//name += getTypeNameFromDINode(ty) + ", ";
				name += getTypeName(ty, function, "") + ", ";
				paramIndex++;
			}
			name += ");";
			return name;
			//}
		} else {
			if (DITypeRef temp = dyn_cast<DIDerivedType>(dt)->getBaseType()) {
				DIType *baseTy = temp.resolve(TypeIdentifierMap);
				return getTypeName(baseTy, function, fieldName);
			}
		/*DIType *baseTy =
		  dyn_cast<DIDerivedType>(dt)->getBaseType().resolve(TypeIdentifierMap);
		  while ((baseTy->getTag() == dwarf::DW_TAG_typedef || baseTy->getTag() == dwarf::DW_TAG_const_type 
		  || baseTy->getTag() == dwarf::DW_TAG_pointer_type || baseTy->getTag() == dwarf::DW_TAG_union_type)) {
		  if (DITypeRef temp = dyn_cast<DIDerivedType>(baseTy)->getBaseType())
		  baseTy = temp.resolve(TypeIdentifierMap);
		  else
		  break;
		  }
		  return baseTy->getName().str();*/
		}
	}

	void offsetPrinter(const DSNode &node, std::ofstream &file, StringRef op,
			offsetNames &of, std::string functionName, std::string indent) {
		DSNode::const_offset_iterator ii, ei;
		//int sz, i=0;
		if (op.str() == "read") {
			file << "\n" << indent << "Read: \n";
			ii = node.read_offset_begin();
			ei = node.read_offset_end();
			//sz = node.read_offset_sz();
		} else if (op.str() == "write") {
			file << "\n" << indent << "Write: \n";
			ii = node.write_offset_begin();
			ei = node.write_offset_end();
			//sz = node.write_offset_sz();
		}
		//errs() << "size: " << sz << "\n";
		for (; ii != ei; ii++) {
			unsigned offset = *ii;
			std::string Name("????");
			if (of.find(offset) != of.end()) {
				Name = of.at(offset).first;
			}
			errs() << indent << " offset: " << *ii << "\t\t" << Name << "\n";
			of.at(offset).second->dump();
			//file << indent << " offset: " << *ii << "\t\t" << getTypeName(of.at(offset).second, functionName, Name) << "\n";
		}
	}

	bool DSAGenerator::runOnModule(Module &m) {

		//include/dsaGenerator/DSAGenerator.h +17 //BU Definition
		//https://github.com/llvm-mirror/poolalloc/blob/master/include/dsa/DataStructure.h line 22
		//http://llvm.org/doxygen/classllvm_1_1Pass.html#a4863e5e463fb79955269fbf7fbf52b80
		BU = &getAnalysis<BUDataStructures>();

		std::error_code EC;
		llvm::raw_fd_ostream F("bu", EC, sys::fs::OpenFlags::F_None);
		BU->print(F, &m);
		if (NamedMDNode *CU_Nodes = m.getNamedMetadata("llvm.dbg.cu")) {
			//include/llvm/IR/DebugInfo.h:37:typedef DenseMap<const MDString *, DIType *> DITypeIdentifierMap;
			//MDString is a A single uniqued string. These are used to efficiently contain a byte sequence for metadata. 
			//DIType (DebugInfo Type) - the base class for types.  
			TypeIdentifierMap = generateDITypeIdentifierMap(CU_Nodes);
		}
		//std::error_code EC;
		//StringRef b("bu");
		//llvm::raw_fd_ostream F1(b, EC, sys::fs::OpenFlags::F_None);
		//std::ofstream F1("bu");
		//BU->print(F1, &m);
		std::ofstream file(m.getName().str() + ".idl");
		std::ofstream undefinedFunctionsFile(m.getName().str() + "_undefined_functions.txt");
		std::ofstream definedFunctionsFile(m.getName().str() + "_defined_functions.txt");
		//std::string functionsListFile = getFunctionsList();
		//std::unordered_set<std::string> functions;

		//functionsListFile is empty, the following is not invoked.
		/*if(!functionsListFile.empty()) {
			std::ifstream functionFile(functionsListFile);
			std::copy(std::istream_iterator<std::string>(functionFile),
					std::istream_iterator<std::string>(),
					std::inserter(functions, functions.begin()));
		}*/

		// scans through all caller callee functions.
		for (auto &F : m) {

			errs() << "Scanning Function {"<<F.getName().str()<<"}\n";

			//ignore functions like llvm.debug.declare
			//http://en.cppreference.com/w/cpp/string/basic_string/npos	
			if (F.getName().find("llvm") == std::string::npos){ 
			//&& (functions.empty() || functions.find(F.getName()) != functions.end())) {

				//Return true if the primary definition of this global value is outside of the current translation unit.
				if (F.isDeclaration()) {
					errs() << "an undefined function{"<< F.getName().str()<<"}\n";
					undefinedFunctionsFile << F.getName().str() << "\n";
					continue;
				}
				else {
					errs() << "a defined function{"<< F.getName().str()<<"}\n";
					definedFunctionsFile << F.getName().str() << "\n";
				}

				//include/dsa/DSGraph.h +194	
				DSGraph *graph = BU->getDSGraph(F);
				
				//include/dsa/DSNode.h +43
				std::vector<DSNode *> argumentNodes;

				//errs() << "has metadata: " << F.hasMetadata() << "\n";

				//include/llvm/ADT/iterator_range.h +32 | args is an iterator range
				//arguments of undefined functions are not available
				for (auto &arg : F.args()) {
					errs() << "Scanning argument {" << arg.getName().str() << "}\n";
					if (arg.hasName()) {
						errs() << arg.getArgNo() << " = arg -->" << arg.getName().str() << "\n";
					}

					//did not find the following case to be true
					else {
						errs() << "arg does not have a name";
					}

					// XXX: What about non-pointer variables ??
					if (arg.getType()->isPointerTy()) {

						//include/dsa/DSSupport.h +54
						DSNodeHandle &nodeHandle = graph->getNodeForValue(&arg);
						DSNode *node = nodeHandle.getNode();
						errs() << "isglobal: " << node->isGlobalNode() << "-------------------\n";
						if (node->isGlobalNode())
							continue;
						std::string structName;
						errs() << "arg.getArgNo(){"<<arg.getArgNo()<<"}\n";
						offsetNames of = getArgFieldNames(F, arg.getArgNo() + 1, arg.getName(), structName);
						dumpOffsetNames(of);
						//file << "collapsed: " << node->isCollapsedNode() << "\n";
						//file << "forward: " << node->isForwarding() << "\n";
						std::vector<DSNode *> visitedNodes;

						visitedNodes.push_back(node);
						printOffsets(node, "", &file, &visitedNodes, of, arg.getType(), arg.getName(), structName, F.getName().str());
					}
					file << "\n";
				}
			}
		}

		NamedMDNode* nmd = m.getNamedMetadata("llvm.dbg.cu");
		if(DICompileUnit* cu = dyn_cast<DICompileUnit>(nmd->getOperand(0))) {
			DIGlobalVariableArray globalVariables = cu->getGlobalVariables();
			DSGraph* graph = BU->getGlobalsGraph();
			for(unsigned int i=0; i<globalVariables.size(); i++) {
				//globalVariables[i]->dump();
				//errs() << globalVariables[i]->getDisplayName().str() << "\n";
				Constant* var = globalVariables[i]->getVariable();
				//var->dump();
				DIType* diType = globalVariables[i]->getType().resolve(TypeIdentifierMap);
				std::string structName;
				offsetNames of;
				getAllNames(diType, of, 0, "", "  ", globalVariables[i]->getDisplayName(), structName);
				//offsetNames of = getArgFieldNames(F, arg.getArgNo() + 1, arg.getName(), structName);
				dumpOffsetNames(of);
				DSNodeHandle &nodeHandle = graph->getNodeForValue(var);
				DSNode *node = nodeHandle.getNode();
				errs() << "name: " << globalVariables[i]->getDisplayName() << "-------------\n";
				errs() << "is global: " << node->isGlobalNode() << "---------------\n";
				//file << "collapsed: " << node->isCollapsedNode() << "\n";
				//file << "forward: " << node->isForwarding() << "\n";
				std::vector<DSNode *> visitedNodes;
				visitedNodes.push_back(node);
				printOffsets(node, "", &file, &visitedNodes, of, var->getType(), globalVariables[i]->getDisplayName(), structName, "bu.global");
				file << "\n";
			}
		}
		return false;
	}

	// Pass ID variable [Not used anywhere]
	char DSAGenerator::ID = 0;
}
