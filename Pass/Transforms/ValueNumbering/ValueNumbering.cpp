#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/IR/CFG.h"

using namespace std;

using namespace llvm;

struct table{
    llvm::Value* var;
    int loadReg;
    StringRef name;
};

struct blockContent{
    StringRef liveIn[10];
    StringRef liveOut[10];
    StringRef ueVar[10];
    StringRef varKill[10];
    llvm::BasicBlock* block;
    StringRef name;
    int blockId;
    llvm::BasicBlock* pred;
    llvm::BasicBlock* succ;
};



struct table hTable[10];
struct blockContent blockCon[10];
int blockCount = -1;
int loadCount = 0;
int tableCount = loadCount;
int ueVarCount = 0;
int varKillCount = 0;
bool loadFound = false;
bool first = true;
bool afterOp = false;

namespace
{
    // This method implements what the pass does
    void visitor(Function &F)
    {
        // Here goes what you want to do with a pass
        string func_name = "test";
        errs() << "ValueNumbering: " << F.getName() << "\n";


        // Comment this line
        if (F.getName() != func_name)
            return;

//////////////////////////////Implementing UEVar, VarKill//////////////////////////////

        for (auto &basic_block : F)
        {
            blockCount++;
            blockCon[blockCount].block = &basic_block;
            blockCon[blockCount].name = basic_block.getName();
            blockCon[blockCount].blockId = blockCount;
            blockCon[blockCount].pred = basic_block.getSinglePredecessor();
            blockCon[blockCount].succ = basic_block.getSingleSuccessor();
            // errs() << blockCon[blockCount].pred << "\n";
            // errs() << blockCon[blockCount].succ << "\n";
            errs() << "2 PREDS? "<<basic_block.hasNPredecessors (2) << "\n";
            
            errs() << blockCon[blockCount].blockId << "\n";
            errs() << blockCon[blockCount].block << "\n";
            errs() << blockCon[blockCount].name << "\n";
            for (auto &inst : basic_block)
            {

                llvm::Value* var1 = inst.getOperand(0); 
                StringRef var1Name = inst.getOperand(0)->getName(); 
                llvm::Value* var2 = inst.getOperand(1);
                StringRef var2Name = inst.getOperand(1)->getName();
                loadFound = false;

                if (inst.getOpcode() == Instruction::Load)
                {

                    if (loadFound == false){
                        hTable[loadCount].var = var1;
                        hTable[loadCount].loadReg = loadCount;
                        hTable[loadCount].name = var1Name;
                        errs() << formatv("{0,-40}", inst) << hTable[loadCount].name << " = " << hTable[loadCount].var << "\n";
                        loadCount++;
                    }
                }
                if (inst.isBinaryOp())
                {
                    if (inst.getOpcode() == Instruction::Add)
                    {
                        blockCon[blockCount].ueVar[ueVarCount] = hTable[loadCount-2].name;
                        blockCon[blockCount].ueVar[ueVarCount+1] = hTable[loadCount-1].name;
                        StringRef name1 = hTable[loadCount-2].name;
                        StringRef name2 = hTable[loadCount-1].name;
                        errs() << formatv("{0,-40}", inst) << "UEVAR: " << name1 << " " << name2 << "\n";
                        ueVarCount += 2;
                        afterOp = true;
                    }
                }
                if (inst.getOpcode() == Instruction::Store)
                {
                    if(afterOp == true){
                        hTable[tableCount].var = var2;
                        hTable[tableCount].name = var2Name;
                        blockCon[blockCount].varKill[ueVarCount] = hTable[tableCount].name;
                        errs() << formatv("{0,-40}", inst) << "VARKILL: " << blockCon[blockCount].varKill[ueVarCount] << "\n";
                        varKillCount++;
                        tableCount++;
                        afterOp = false;
                    }else{
                        blockCon[blockCount].ueVar[ueVarCount] = hTable[loadCount-1].name;
                        blockCon[blockCount].varKill[ueVarCount] = hTable[loadCount-2].name;
                        errs() << formatv("{0,-40}", inst) << "UEVAR: " << blockCon[blockCount].ueVar[ueVarCount] << "\n";
                        errs() << formatv("{0,-40}", inst) << "VARKILL: " << blockCon[blockCount].varKill[ueVarCount] << "\n";
                        ueVarCount++;
                        varKillCount++;
                        tableCount++;
                    }
                }
            }
        }


//////////////////////////////Implementing LiveOut//////////////////////////////
        // for(int i=blockCount ; i > 0 ; i--){
        //     if (first == true){
        //         first == false;
        //     }else{

        //         blockCon[blockCount].liveOut =
        //     }
        //     // blockCon[blockCount].liveOut =

        // }



        for (auto &basic_block : F)
        {
            for (BasicBlock *Pred : predecessors(&basic_block)) {

                errs() << basic_block << "\n";
                errs() << Pred->getName() << "\n";
                // for(int i=0 ; i < (sizeof(blockCon[blockCount].ueVar)/sizeof(blockCon[blockCount].ueVar[0])) ; i++){
                //     if(blockCon[blockCount].ueVar[i] != ""){
                //         errs() << "ueVar: " << blockCon[blockCount].ueVar[i] << "\n";
                //     }
                // }
                // // errs() << "ueVar: " << blockCon[blockCount].ueVar << "\n";
                // // errs() << "varKill: " << blockCon[blockCount].varKill << "\n";
                // blockCount--;
            }
        }



        for (auto &basic_block : F)
        {
            for (auto &inst : basic_block)
            {

                if (inst.getOpcode() == Instruction::Load)
                {
                }
                if (inst.getOpcode() == Instruction::Store)
                {
                    
                }
                if (inst.isBinaryOp())
                {
                    if (inst.getOpcode() == Instruction::Add)
                    {

                    }
                    if (inst.getOpcode() == Instruction::Mul)
                    {

                    }
                    if (inst.getOpcode() == Instruction::Sub)
                    {

                    }
                    if (inst.getOpcode() == Instruction::UDiv)
                    {

                    }


                    // see other classes, Instruction::Sub, Instruction::UDiv, Instruction::SDiv
                    // errs() << "Operand(0)" << (*inst.getOperand(0))<<"\n";
                    auto* ptr = dyn_cast<User>(&inst);
                    //errs() << "\t" << *ptr << "\n";
                    for (auto it = ptr->op_begin(); it != ptr->op_end(); ++it)
                    {
                        errs() << "\t" << *(*it) << "\n";
                        // llvm::User* instr = dyn_cast<User>(it); 
                        // if ((*it)->hasName())
                        // errs() << (*it)->getName() << "\n";
                    }
                } // end if
            }     // end for inst
        }         // end for block
    }

    // New PM implementation
    struct ValueNumberingPass : public PassInfoMixin<ValueNumberingPass>
    {

        // The first argument of the run() function defines on what level
        // of granularity your pass will run (e.g. Module, Function).
        // The second argument is the corresponding AnalysisManager
        // (e.g ModuleAnalysisManager, FunctionAnalysisManager)
        PreservedAnalyses run(Function &F, FunctionAnalysisManager &)
        {
            visitor(F);
            return PreservedAnalyses::all();
        }

        static bool isRequired() { return true; }
    };
}

//-----------------------------------------------------------------------------
// New PM Registration
//-----------------------------------------------------------------------------
extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK
llvmGetPassPluginInfo()
{
    return {
        LLVM_PLUGIN_API_VERSION, "ValueNumberingPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB)
        {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>)
                {
                    if (Name == "value-numbering")
                    {
                        FPM.addPass(ValueNumberingPass());
                        return true;
                    }
                    return false;
                });
        }};
}
