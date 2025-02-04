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
bool tryReplaceRoundTrip(IntToPtrInst* intToPtr, IRBuilder<>& builder) {
    bool changed = false;

    Instruction* op_inst = dyn_cast<Instruction>(intToPtr->getOperand(0));

    if (op_inst->getNumUses() > 1) {
      // cannot safely replace
      errs() << "More than 1 uses from from add\n";
      return changed;
    }

    PtrToIntInst* ptrToInt = dyn_cast<PtrToIntInst>(op_inst->getOperand(0));

    if (ptrToInt->getNumUses() > 1) {
      // cannot safely replace
      errs() << "More than 1 uses from from ptrToInt\n";
      return changed;
    }

    Value* ptr = ptrToInt->getOperand(0);
    Instruction* gep = GetElementPtrInst::Create(builder.getInt8Ty(), 
                                                 ptr, 
                                                 ArrayRef<Value*>{op_inst->getOperand(1)}, 
                                                 "res", 
                                                 intToPtr->getNextNode());

    intToPtr->replaceAllUsesWith(gep);
    intToPtr->eraseFromParent();
    op_inst->eraseFromParent();
    ptrToInt->eraseFromParent();

    changed = true;
    
    return changed;
  }

  PreservedAnalyses run(Function& F, FunctionAnalysisManager&) {
    IRBuilder<> builder(F.getContext());
    bool changed = false;
    
    for (Instruction& Inst : instructions(F)) {
      IntToPtrInst* intToPtr = dyn_cast<IntToPtrInst>(&Inst);

      if (!intToPtr) {
        continue;
      }

      errs() << "Found intToPtr\n";

      Instruction* op_inst = dyn_cast<Instruction>(intToPtr->getOperand(0));

      if (!op_inst) {
        continue;
      }

      changed |= tryReplaceRoundTrip(intToPtr, builder);

      verifyFunction(F, &errs());

      if (changed) break;
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