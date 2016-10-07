//
// Copyright (c) 2013 Pantazis Deligiannis (p.deligiannis@imperial.ac.uk)
// This file is distributed under the MIT License. See LICENSE for details.
//

#include "llvm/LinkAllPasses.h"
#include "llvm/PassManager.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"

#include "dsaGenerator/DSAGenerator.h"

#define DEBUG_TYPE "dsa"

static llvm::cl::opt<std::string>
InputFilename(llvm::cl::Positional, llvm::cl::desc("<input LLVM bitcode file>"),
  llvm::cl::Required, llvm::cl::value_desc("filename"));

static llvm::cl::opt<std::string>
DefaultDataLayout("default-data-layout", llvm::cl::desc("data layout string to use if not specified by module"),
  llvm::cl::init(""), llvm::cl::value_desc("layout-string"));

namespace {
  using namespace llvm;
  static void check(std::string E) {
    if (!E.empty()) {
      if (errs().has_colors())
        errs().changeColor(raw_ostream::RED);
      errs () << E << "\n";
      if (errs().has_colors())
        errs().resetColor();
      exit(1);
    }
  }
}

int main(int argc, char **argv) {
  llvm::llvm_shutdown_obj shutdown;  // calls llvm_shutdown() on exit
  llvm::cl::ParseCommandLineOptions(argc, argv, "dsaGenerator\n");

  llvm::sys::PrintStackTraceOnErrorSignal();
  llvm::PrettyStackTraceProgram PSTP(argc, argv);
  llvm::EnableDebugBuffering = true;

  llvm::SMDiagnostic err;
  std::unique_ptr<llvm::Module> module = llvm::parseIRFile(InputFilename, err, llvm::getGlobalContext());
  if (!err.getMessage().empty())
    check("Problem reading input bitcode/IR: " + err.getMessage().str());

  auto &L = module.get()->getDataLayoutStr();
  if (L.empty())
    module.get()->setDataLayout(DefaultDataLayout);

  ///////////////////////////////
  // initialise and run passes //
  ///////////////////////////////

  llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
  llvm::initializeAnalysis(Registry);

  llvm::PassManager pass_manager;

  pass_manager.add(new llvm::DataLayoutPass());
  pass_manager.add(new dsa::DSAGenerator());
   
  pass_manager.run(*module.get());

  return 0;
}
