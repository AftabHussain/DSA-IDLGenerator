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
	void offsetPrinter(const DSNode &node, std::ofstream &file, StringRef op, offsetNames &of, std::string, std::string);
	void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off, std::string baseName, std::string indent, StringRef argName, std::string&);
	std::string moduleName = "[dsa-gen]";

	/// Prints it in console. Solely for debugging purposes
	void dumpOffsetNames(offsetNames &of) {
		std::string printinfo = moduleName + "[dumpOffsetNames]: ";
		// errs() << printinfo<<"Entered function\n";
		for (auto off : of) {
			errs() << printinfo<< "offset : " << off.first << "\n";
			errs() << printinfo<< "name : " << std::get<0>(off.second) << "\n";
		    if (std::get<0>(off.second)=="Block")errs()<<"END WATCH\n";
		}
	}

	void printOffsets(DSNode *node, std::string indentation, std::ofstream *file,
			std::vector<DSNode *> *visitedNodes, offsetNames &of2, 
			Type* elementType, StringRef elementName, std::string& structName, std::string functionName) {
		
		std::string printinfo = moduleName + "[printOffsets]: ";

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

			/* Class to represent struct types. 
			There are two different kinds of struct types: Literal structs and Identified structs.
			http://llvm.org/doxygen/classllvm_1_1StructType.html#details*/

			while(!isa<StructType>(argType)) {
				argType = dyn_cast<PointerType>(argType)->getElementType();
			//*file << indentation << "\nargument " << dyn_cast<PointerType>(argType)<< "> " <<" {\n";
			}

			/*substr(7) because names begins with "struct." --- 7 characters
			projection <struct structure_name> function_name.argument name*/	
			// *file << indentation << "\nprojection <struct " << dyn_cast<StructType>(argType)->getName().substr(7).str() << "> " << functionName << "." << elementName.str() << " {\n";
			
			//Condensed idl
			*file << "Function: " << functionName <<"\n";
			*file << "Projects structure: " << dyn_cast<StructType>(argType)->getName().substr(7).str()  <<"\n";
			// *file << indentation << "\nprojection <struct " << dyn_cast<StructType>(argType)->getName().substr(7).str() << "> " << functionName << "." << elementName.str() << " {\n";
                        
			//*file << (node->isCollapsedNode() ? "collapsed\n" : "not collapsed\n");
			offsetPrinter(*node, *file, "read", of2, functionName, indentation);
			offsetPrinter(*node, *file, "write", of2, functionName, indentation);
			*file << "\n\n";
			// *file << indentation << "}\n\n";
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
				errs() << printinfo<<"OUT of range exception " << e.what() << "\n";
                                errs() <<printinfo<< "offset: " << offset << "\n";
				continue;
			}
			
			DIType *Ty = of2.at(offset).second;
			//errs() << Ty->getTag() << "\n";
                        structName = "";
			// Previous offset is zero. No continuation
			getAllNames(Ty, ofInner, 0, offset_name, " ", elementName, structName);
			errs() << printinfo<<"CALL dumpOffsetNames on line 130\n";
			dumpOffsetNames(ofInner);
			//assuming that we will have to print this projection only once
			std::string emptyString("");
			printOffsets((*ei).second.getNode(), ind2, file, visitedNodes, ofInner, elementType, elementName, emptyString, emptyString);
		}
	}

	DIType* getLowestDINode(DIType* Ty) {

		std::string printinfo = moduleName + "[getLowestDINode]: ";
		if (Ty->getTag() == dwarf::DW_TAG_pointer_type ||
				Ty->getTag() == dwarf::DW_TAG_member || Ty->getTag() == dwarf::DW_TAG_typedef) {
			DIType *baseTy =
				dyn_cast<DIDerivedType>(Ty)->getBaseType().resolve(TypeIdentifierMap);
			if (!baseTy) {
				errs() <<printinfo<< "Type : NULL - Nothing more to do\n";
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

	/*Recursive Fn called by getArgFieldNames*/
	void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off,
			std::string baseName, std::string indent, StringRef argName, std::string& structName) {

		std::string printinfo = moduleName + "[getAllNames]: ";
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
				errs()<< printinfo <<"type information:  "<<der->getBaseType().resolve(TypeIdentifierMap)->getTag()<<"\n"; 

				errs()<< printinfo << "Updating [of] on line 192 with following pair:\n";
				errs()<< printinfo << "first item [new_name] "<<new_name<<"\n";
				of[offset + prev_off] = std::pair<std::string, DIType *>(
						new_name, der->getBaseType().resolve(TypeIdentifierMap));
				/// XXX: crude assumption that we want to peek only into those members
				/// whose sizes are greater than 8 bytes
				if (((der->getSizeInBits() >> 3) > 1) 
						&& der->getBaseType().resolve(TypeIdentifierMap)->getTag()) {
					std::string tempStructName("");
					errs()<< printinfo <<"RECURSIVELY CALL getAllNames on 200\n";
					getAllNames(dyn_cast<DIType>(der), of, prev_off + offset,
							new_name, indent, argName, tempStructName);
				}
				//errs() << "--------------- " << der->getName().str() << "\n";
			}
		} else if (DIBasicType *bas = dyn_cast<DIBasicType>(baseTy)) {
			structName = "";
			//if type tag for the parameter is of pointer_type and DI type is DIBasicType 
			//then treat it as a pointer of native type
			errs()<< printinfo << "Updating [of] on line 209 with following pair:\n";
			errs()<< printinfo << "first item [argName.str()] "<<argName.str()<<"\n";
			// of[0] = std::pair<std::string, DIType *>(argName.str(), bas);//This may be overwriting the correct name of the first member of the structure.
			//TODO need to see whether this case (this else if situation) needs to be handled at all.
		} else {
			structName = "";
		}
	}

	/*Called by runOnModule - Gets the field names of a structure*/
	offsetNames getArgFieldNames(Function &F, unsigned argNumber, StringRef argName, std::string& structName) {


		std::string printinfo = moduleName + "[getArgFieldNames]: ";

		// errs() << printinfo << "F.getName(){"<<F.getName()<<"}\n";
		//errs() << "number of arguments";
		offsetNames offNames;

		//it should be assert((argNumber != 0)....
		//assert((argNumber == 0) && "Request for return type information. Not supported");
	
		//didn't find any such case
		if (argNumber > F.arg_size()) {
			errs() << printinfo << "### WARN : requested data for non-existent element\n";
			return offNames;
		}
		// errs() << moduleName << " getArgFieldNames - "<< "## Function ## : " << F.getName().str()
		// << " | argsize: " << F.arg_size() << " :: Requested " << argNumber
		// << "\n";
		//Google smallvector llvm
		SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
		F.getAllMetadata(MDs);
		for (auto &MD : MDs) {
			if (MDNode *N = MD.second) {
				// errs()<<*N<<" value of the metadata\n";
				//http://llvm.org/docs/ProgrammersManual.html#the-isa-cast-and-dyn-cast-templates
				//If the type of the metadata value can be casted to a DISubprogram
				if (auto *subRoutine = dyn_cast<DISubprogram>(N)->getType()) {
					
					//XXX:if a function takes in no arguments, how can we assume it is of type void?
					if (!subRoutine->getTypeArray()[0]) {
						errs() << printinfo << "return type \"void\" for Function : " << F.getName().str()<< "\n";
					}

					const auto &TypeRef = subRoutine->getTypeArray();

					/// XXX: When function arguments are coerced in IR, the corresponding
					/// debugInfo extracted for that function from the source code will
					/// not have the same number of arguments. Check the indexes to
					/// prevent array out of bounds exception (segfault)

					//did not encounter this case with dummy.c
					if (argNumber >= TypeRef.size()) {
						errs() << printinfo << "TypeArray request out of bounds. Are parameters coerced??\n";
						goto done;
					}

					if (const auto &ArgTypeRef = TypeRef[argNumber]) {
						// Resolve the type
						DIType *Ty = ArgTypeRef.resolve(TypeIdentifierMap);
						// Handle Pointer type
						if (F.getName()=="passF")errs()<<"BEGIN WATCH\n";
						errs() << printinfo <<"CALL getAllNames on line 266 with these params:\n";
						errs() << printinfo << "argName = "<<argName<<"\n";
						errs() << printinfo << "structName = "<<structName<<"\n";
						getAllNames(Ty, offNames, 0, "", "  ", argName, structName);
				}
				}
			}
		}	
		done:
		return offNames;
	}

	/*Called by getTypeName*/	
	std::string getTypeNameFromDINode(DIType* dt) {
		std::string printinfo = moduleName + "[getTypeNameFromDINode]: ";

		while(dt->getName().str() == "") {
			dt = dyn_cast<DIDerivedType>(dt)->getBaseType().resolve(TypeIdentifierMap);
		}
		return dt->getName().str();
	}

	/*Recursive Function - first called by offsetPrinter*/
	std::string getTypeName(DIType* dt, std::string function, std::string fieldName) {
		

		std::string printinfo = moduleName + "[getTypeName]: ";

		errs()<<printinfo<<"check type name\n"<<dt->getName().str();
		if (DIBasicType* bt = dyn_cast<DIBasicType>(dt)) {
			return bt->getName().str() + " " + fieldName ;//fieldName returns the struct var name itself instead of its first field var.
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

	/*Called by runOnModule*/
	void offsetPrinter(const DSNode &node, std::ofstream &file, StringRef op,
			offsetNames &of, std::string functionName, std::string indent) {
		
		std::string printinfo = moduleName + "[offsetPrinter]: ";
		
		DSNode::const_offset_iterator ii, ei;
		//int sz, i=0;
		if (op.str() == "read") {
			file <<  "Read: \n";
			// file << "\n" << indent << "Read: \n";
			ii = node.read_offset_begin();
			ei = node.read_offset_end();
			//sz = node.read_offset_sz();
		} else if (op.str() == "write") {
			file <<  "Write: \n";
			// file << "\n" << indent << "Write: \n";
			ii = node.write_offset_begin();
			ei = node.write_offset_end();
			//sz = node.write_offset_sz();
		}
		//errs() << "size: " << sz << "\n";
		// errs() << "checking ii ei"<< "\n";
		// errs() << *ii << " " << *ei << "\n" ;
		
		for (; ii != ei; ii++) {
			unsigned offset = *ii;
			std::string Name("????");
			if (of.find(offset) != of.end()) {
				Name = of.at(offset).first;
			}
		//	errs() << indent << " offsetT: " << *ii << "\t\t" << Name << "\n";//for debugging
		//	file << indent << " offsetT: " << *ii << "\t\t" << Name << "\n";//for debugging
			of.at(offset).second->dump();

			//TODO THIS IS THE LINE THAT WAS CAUSING A DOUBLE FREE ERROR WHILE PROCESSING NET.O.BC (NEED TO SEE THIS CODE LATER, WHETHER WE NEED IT AT ALL)
			//MORE INFO: found if this line is not used. net.o.bc.idl is blank, however, dummy.bc.idl is still generated (has content, i.e. projections, without checking for 
			//their correctness at the moment--This prints the offsets in the idl file.)
			//when only printing *ii with net.bc, idl file (the file printing projections) only prints upto projection sock_sendpage.file, then only partially prints the constructs of the next projection.
			//also no double free error was generated.
			//More Development: the function getTypeName(of.at(offset).second, functionName, Name) generates the double free error, and also generates the garbage vaulues in the idl file for net.bc
			file << indent << " offset: " << "\t\t" << getTypeName(of.at(offset).second, functionName, Name) << " see this "<<of.at(offset).first<< "\n";//Name incorrectly holds the structure var name instead of structure member name //TODO: need to fix this
		//	file << indent << " offset: " << "\t\t" ;
		//	file << indent << " offset: " << *ii <<"\n";//<< "\t\t" << getTypeName(of.at(offset).second, functionName, Name) << "\n";
			//file << indent << " offset: " << *ii << "\t\t" << getTypeName(of.at(offset).second, functionName, Name) << "\n";
		}
	}

	bool DSAGenerator::runOnModule(Module &m) {
		std::string printinfo = moduleName + "[runOnModule]: ";
		errs()<<printinfo<<"Running Pass DSAGenerator \n";
		/*include/dsaGenerator/DSAGenerator.h +17 //BU Definition //pointer to BUDataStructures
		https://github.com/llvm-mirror/poolalloc/blob/master/include/dsa/DataStructure.h line 22
		http://llvm.org/doxygen/classllvm_1_1Pass.html#a4863e5e463fb79955269fbf7fbf52b80
		BUDataStructures - The analysis that computes the interprocedurally closed
 		data structure graphs for all of the functions in the program.  This pass
		only performs a "Bottom Up" propagation (hence the name).
		https://github.com/llvm-mirror/poolalloc/blob/eb3a28cc226248240eb05273f543aca074979930/include/dsa/DataStructure.h#L218
		*/
		// BU = &getAnalysis<BUDataStructures>();
		TD = &getAnalysis<TDDataStructures>();


		std::error_code EC;

		/*Open the specified file for writing.
		If an error occurs, information about the error is put into EC, 
		and the stream should be immediately destroyed; Flags allows optional flags to control how the file will be opened.
		*/
		llvm::raw_fd_ostream F("bu", EC, sys::fs::OpenFlags::F_None);


		/*prints the analysis results - a function of BU's parent class - DataStructures in poolalloc*/
		// BU->print(F, &m);
		TD->print(F, &m);

		// Despite its name, a NamedMDNode isn't itself an MDNode. NamedMDNodes belong to modules, have names, and contain lists of MDNodes.
		if (NamedMDNode *CU_Nodes = m.getNamedMetadata("llvm.dbg.cu")) {

			/*include/llvm/IR/DebugInfo.h:37:typedef DenseMap<const MDString *, DIType *> DITypeIdentifierMap;
			MDString is a A single uniqued string. These are used to efficiently contain a byte sequence for metadata. 
			DIType (DebugInfo Type) - the base class for types.*/  
			TypeIdentifierMap = generateDITypeIdentifierMap(CU_Nodes);

		}

		/* Unused Code
		std::error_code EC;
		StringRef b("bu");
		llvm::raw_fd_ostream F1(b, EC, sys::fs::OpenFlags::F_None);
		std::ofstream F1("bu");
		BU->print(F1, &m);
		*/

		std::ofstream file(m.getName().str() + ".idl");
		std::ofstream undefinedFunctionsFile(m.getName().str() + "_undefined_functions.txt");
		std::ofstream definedFunctionsFile(m.getName().str() + "_defined_functions.txt");

		// Unused Code	
		std::string functionsListFile = getFunctionsList();
		errs()<<printinfo<<"output of getFunctionsList(): "<<getFunctionsList()<<" \n";
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

			errs() <<printinfo<< "Scanning Function {"<<F.getName().str()<<"}\n";

			//ignore functions like llvm.debug.declare
			//http://en.cppreference.com/w/cpp/string/basic_string/npos	
			if (F.getName().find("llvm") == std::string::npos){ 
			/*Unused condition && (functions.empty() || functions.find(F.getName()) != functions.end())) {*/

				//Return true if the primary definition of this global value is outside of the current translation unit.
				if (F.isDeclaration()) {
					errs()<<printinfo << "an undefined function{"<< F.getName().str()<<"}\n";
					undefinedFunctionsFile << F.getName().str() << "\n";
					continue;
				}
				else {
					errs()<<printinfo << "a defined function{"<< F.getName().str()<<"}\n";
					definedFunctionsFile << F.getName().str() << "\n";
				}

				//-----The following happens only for defined functions-----//
				
				/*include/dsa/DSGraph.h +194	
				The graph that represents a function.*/
				// DSGraph *graph = BU->getDSGraph(F);
				DSGraph *graph = TD->getDSGraph(F);
				
				/*include/dsa/DSNode.h +43
				DSNode - Data structure node class
 				This class represents an untyped memory object of Size bytes.  It keeps
    				track of any pointers that have been stored into the object as well as the
    				different types represented in this object.*/
				std::vector<DSNode *> argumentNodes;

				//errs() << "has metadata: " << F.hasMetadata() << "\n";

				//include/llvm/ADT/iterator_range.h +32 | args is an iterator range
				for (auto &arg : F.args()) {
					//if (arg.getType()->isFPOrFPVectorTy()){errs()<<"got function type! arg "<<arg.getName().str()<<"\n";}
					errs()<<printinfo << "Scanning argument {" << arg.getName().str() << "}\n";
					if (arg.hasName()) {
						errs()<<printinfo << arg.getArgNo() << " = arg -->" << arg.getName().str() << "\n";
					}

					//did not find the following case to be true
					else {
						errs()<<printinfo << "arg does not have a name";
					}

					// XXX: What about non-pointer variables ??
					if (arg.getType()->isPointerTy()) {

						/* include/dsa/DSSupport.h +54
						DSNodeHandle - Implement a "handle" to a data structure node that takes care
						of all of the add/un'refing of the node to prevent the backpointers in the
						graph from getting out of date.  This class represents a "pointer" in the
						graph, whose destination is an indexed offset into a node.
						getNodeForValue - Given a value that is used or defined in the body of the
  						current function, return the DSNode that it points to. */
						DSNodeHandle &nodeHandle = graph->getNodeForValue(&arg);
						DSNode *node = nodeHandle.getNode();
						errs()<<printinfo << "is the node for this argument global?: " << node->isGlobalNode() << "-------------------\n";
						if (node->isGlobalNode())
							continue;
						std::string structName;
						errs()<<printinfo << "arg.getArgNo(){"<<arg.getArgNo()<<"}\n";
						errs() << printinfo<<"CALL getArgFieldNames on line 521 with these parameters:\n";
						errs() << printinfo<<"F, s.t F.getName() = "<<F.getName()<<"\n";
						errs() << printinfo<<"arg.getArgNo() + 1 = "<<arg.getArgNo() + 1<<"\n";
						errs() << printinfo<<"arg.getName() = "<<arg.getName()<<"\n";
						errs() << printinfo<<"structName = "<<structName<<"\n";
						offsetNames of = getArgFieldNames(F, arg.getArgNo()+1, arg.getName(), structName);
						errs() << printinfo<<"CALL dumpOffsetNames on line 523\n";
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

		/*A tuple of MDNodes.*/
		NamedMDNode* nmd = m.getNamedMetadata("llvm.dbg.cu");
		/*About Dynamic cast operator and llvm's dyn_cast 
		http://llvm.org/docs/ProgrammersManual.html#the-isa-cast-and-dyn-cast-templates
		http://www.bogotobogo.com/cplusplus/dynamic_cast.php*/	
		if(DICompileUnit* cu = dyn_cast<DICompileUnit>(nmd->getOperand(0))) {
			DIGlobalVariableArray globalVariables = cu->getGlobalVariables();
			// DSGraph* graph = BU->getGlobalsGraph();
			DSGraph* graph = TD->getGlobalsGraph();
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
				errs() << printinfo<<"CALL dumpOffsetNames on line 554\n";
				dumpOffsetNames(of);
				DSNodeHandle &nodeHandle = graph->getNodeForValue(var);
				DSNode *node = nodeHandle.getNode();
				errs() <<printinfo<< "name: " << globalVariables[i]->getDisplayName() << "\n";
				errs() <<printinfo<< "is global: " << node->isGlobalNode() << "\n";
				//file << "collapsed: " << node->isCollapsedNode() << "\n";
				//file << "forward: " << node->isForwarding() << "\n";
				std::vector<DSNode *> visitedNodes;
				visitedNodes.push_back(node);
				// printOffsets(node, "", &file, &visitedNodes, of, var->getType(), globalVariables[i]->getDisplayName(), structName, "bu.global");
				// file << "\n";
				printOffsets(node, "", &file, &visitedNodes, of, var->getType(), globalVariables[i]->getDisplayName(), structName, "td.global");
				file << "\n";
			}
		}
		return false;
	}

	// Unused code - Pass ID variable 
	char DSAGenerator::ID = 0;
}
