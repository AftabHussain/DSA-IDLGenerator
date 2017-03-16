#
# This file is distributed under the MIT License. See LICENSE for details.
#

cmake_minimum_required(VERSION 2.8)
project(dsa)

if (NOT WIN32 OR MSYS OR CYGWIN)
  find_program(LLVM_CONFIG_EXECUTABLE NAMES llvm-config PATHS ${LLVM_CONFIG} DOC "llvm-config")

  if (LLVM_CONFIG_EXECUTABLE STREQUAL "LLVM_CONFIG_EXECUTABLE-NOTFOUND")
    message(FATAL_ERROR "llvm-config could not be found!")
  endif()

  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --cxxflags
    OUTPUT_VARIABLE LLVM_CXXFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  set(LLVM_CXXFLAGS "${LLVM_CXXFLAGS} -fexceptions -fno-rtti")

  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --libs
    OUTPUT_VARIABLE LLVM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs
    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  execute_process(
    COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags
    OUTPUT_VARIABLE LLVM_LDFLAGS
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

else()
  set(LLVM_SRC "" CACHE PATH "LLVM source directory")
  set(LLVM_BUILD "" CACHE PATH "LLVM build directory")
  set(LLVM_BUILD_TYPE "" CACHE STRING "LLVM build type")

  if (NOT EXISTS "${LLVM_SRC}/include/llvm")
    message(FATAL_ERROR "Invalid LLVM source directory: ${LLVM_SRC}")
  endif()

  set(LLVM_LIBDIR "${LLVM_BUILD}/lib/${LLVM_BUILD_TYPE}")
  if (NOT EXISTS "${LLVM_LIBDIR}")
    message(FATAL_ERROR "Invalid LLVM build directory: ${LLVM_BUILD}")
  endif()

  set(LLVM_CXXFLAGS "\"/I${LLVM_SRC}/include\" \"/I${LLVM_BUILD}/include\" -D_SCL_SECURE_NO_WARNINGS -wd4146 -wd4244 -wd4355 -wd4482 -wd4800")
  set(LLVM_LDFLAGS "")
  set(LLVM_LIBS "${LLVM_LIBDIR}/LLVMTransformUtils.lib" "${LLVM_LIBDIR}/LLVMipa.lib" "${LLVM_LIBDIR}/LLVMAnalysis.lib" "${LLVM_LIBDIR}/LLVMTarget.lib" "${LLVM_LIBDIR}/LLVMMC.lib" "${LLVM_LIBDIR}/LLVMObject.lib" "${LLVM_LIBDIR}/LLVMBitReader.lib" "${LLVM_LIBDIR}/LLVMCore.lib" "${LLVM_LIBDIR}/LLVMSupport.lib")

endif()

include_directories(include)

add_library(DSAGenerator STATIC
  include/dsaGenerator/DSAGenerator.h
  include/dsaGenerator/UndefinedFunctionsPass.h
  lib/dsaGenerator/DSAGenerator.cpp
  lib/dsaGenerator/UndefinedFunctionsPass.cpp
)

add_library(dsa STATIC
  include/dsa/AddressTakenAnalysis.h
  include/dsa/DSCallGraph.h
  include/dsa/DSNode.h
  include/dsa/EntryPointAnalysis.h
  include/dsa/keyiterator.h
  include/dsa/svset.h
  include/dsa/AllocatorIdentification.h
  include/dsa/DSGraph.h
  include/dsa/DSSupport.h
  include/dsa/stl_util.h
  include/dsa/CallTargets.h
  include/dsa/DSGraphTraits.h
  include/dsa/DataStructure.h
  include/dsa/TypeSafety.h
  include/dsa/super_set.h
  include/dsa/DSMonitor.h
  lib/DSA/AddressTakenAnalysis.cpp
  lib/DSA/CallTargets.cpp
  lib/DSA/DSTest.cpp
  lib/DSA/EquivClassGraphs.cpp
  lib/DSA/StdLibPass.cpp
  lib/DSA/AllocatorIdentification.cpp
  lib/DSA/CompleteBottomUp.cpp
  lib/DSA/DataStructure.cpp
  lib/DSA/GraphChecker.cpp
  lib/DSA/Printer.cpp
  lib/DSA/TopDownClosure.cpp
  lib/DSA/Basic.cpp
  lib/DSA/DSCallGraph.cpp
  lib/DSA/DataStructureStats.cpp
  lib/DSA/TypeSafety.cpp
  lib/DSA/BottomUpClosure.cpp
  lib/DSA/DSGraph.cpp
  lib/DSA/EntryPointAnalysis.cpp
  lib/DSA/DSMonitor.cpp
  lib/DSA/Local.cpp
  lib/DSA/SanityCheck.cpp
)

add_executable(dsaGenerator
  tools/dsaGenerator/dsaGenerator.cpp
)

set_target_properties(dsaGenerator dsa DSAGenerator
    PROPERTIES COMPILE_FLAGS "${LLVM_CXXFLAGS}")

target_link_libraries(dsa DSAGenerator ${LLVM_LIBS} ${LLVM_SYSTEM_LIBS} ${LLVM_LDFLAGS})
target_link_libraries(DSAGenerator dsa)
target_link_libraries(dsaGenerator dsa DSAGenerator)

INSTALL(TARGETS dsaGenerator
  RUNTIME DESTINATION bin
)
