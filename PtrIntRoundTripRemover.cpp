#include "llvm/IR/PassManager.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

// New PM pass
struct PtrIntRoundTripRemover : public PassInfoMixin<PtrIntRoundTripRemover> {
  bool tryReplaceRoundTrip(IntToPtrInst* intToPtr) {
    if (!intToPtr) return false;

    Instruction* op_inst = dyn_cast<Instruction>(intToPtr->getOperand(0));
    if (!op_inst || op_inst->getOpcode() != Instruction::Add || op_inst->getNumUses() > 1) {
        errs() << "More than 1 use of add result\n";
        return false;
    }

    PtrToIntInst* ptrToInt = dyn_cast<PtrToIntInst>(op_inst->getOperand(0));
    if (!ptrToInt || ptrToInt->getNumUses() > 1) {
        errs() << "More than 1 use of ptrToInt result\n";
        return false;
    }

    IRBuilder<> B(intToPtr);

    Value* gep = B.CreateGEP(B.getInt8Ty(), 
                             ptrToInt->getOperand(0), 
                             {op_inst->getOperand(1)}, 
                             "");

    intToPtr->replaceAllUsesWith(gep);
    intToPtr->eraseFromParent();
    op_inst->eraseFromParent();
    ptrToInt->eraseFromParent();

    return true;
  }

  PreservedAnalyses run(Function& F, FunctionAnalysisManager&) {
    bool changed = false;

    for (auto it = instructions(F).begin(), end = instructions(F).end(); it != end;) {
        Instruction& Inst = *it++;
        if (auto* intToPtr = dyn_cast<IntToPtrInst>(&Inst)) {
            changed |= tryReplaceRoundTrip(intToPtr);
        }
    }

    F.print(outs(), nullptr);
    return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
  }


  static bool isRequired() { return true; }
};


// Register new PM
PassPluginLibraryInfo getPtrIntRoundTripRemoverPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "PtrIntRoundTripRemover", LLVM_VERSION_STRING,
          [](PassBuilder& PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "ptr-int-round-trip-remover") {
                    FPM.addPass(PtrIntRoundTripRemover());
                    return true;
                  }
                  return false;
                });
          }};
}

extern "C" LLVM_ATTRIBUTE_WEAK ::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getPtrIntRoundTripRemoverPluginInfo();
}