#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/IR/CFG.h"
#include <set>

using namespace std;

using namespace llvm;

struct table{
    llvm::Value* var;
    int loadReg;
    StringRef name;
};

struct blockContent{
    std::vector<StringRef> liveOut;
    std::vector<StringRef> ueVar;
    std::vector<StringRef> varKill;
    llvm::BasicBlock* block;
    StringRef name;
    int blockId;
    llvm::BasicBlock* succ[10];
};


struct table hTable[10];
struct blockContent blockCon[10];
int blockCount = -1;
int loadCount = 0;
int tableCount = loadCount;
int ueVarCount = 0;
int varKillCount = 0;
bool loadFound = false;
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
                        blockCon[blockCount].ueVar.insert(blockCon[blockCount].ueVar.end(),hTable[loadCount-2].name);
                        blockCon[blockCount].ueVar.insert(blockCon[blockCount].ueVar.end(),hTable[loadCount-1].name);
                        errs() << formatv("{0,-40}", inst) << "UEVAR: " << hTable[loadCount-2].name << " " << hTable[loadCount-1].name << "\n";
                        ueVarCount += 2;
                        afterOp = true;
                    }
                }
                if (inst.getOpcode() == Instruction::Store)
                {
                    if(afterOp == true){
                        hTable[tableCount].var = var2;
                        hTable[tableCount].name = var2Name;
                        blockCon[blockCount].varKill.insert(blockCon[blockCount].varKill.end(),hTable[loadCount].name);
                        errs() << formatv("{0,-40}", inst) << "VARKILL: " << hTable[tableCount].name << "\n";
                        varKillCount++;
                        tableCount++;
                        afterOp = false;
                    }else{
                        blockCon[blockCount].ueVar.insert(blockCon[blockCount].ueVar.end(),hTable[loadCount-1].name);
                        blockCon[blockCount].varKill.insert(blockCon[blockCount].varKill.end(),hTable[loadCount-2].name);
                        errs() << formatv("{0,-40}", inst) << "UEVAR: " << hTable[loadCount-1].name << "\n";
                        errs() << formatv("{0,-40}", inst) << "VARKILL: " << hTable[loadCount-2].name << "\n";
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

        errs() << "//////////////////////////////Implementing LiveOut//////////////////////////////" << "\n";
        // std::vector<StringRef> v1; 
        // std::vector<StringRef> v2 = {"3f", "4g", "5h", "6i", "7j"}; 
        // std::vector<StringRef> dest1; // create a third vector to write results
        // v1.insert(v1.end(),"concac");
        // v1.insert(v1.end(),"vkl");
        // std::set_union(v1.begin(), v1.end(), v2.begin(), v2.end(),std::back_inserter(dest1));
        // for(int i=0 ; i < 8; i++){
        //     errs() << "dest1    " << dest1[i] <<"\n";
        // }


        // int succLoop=0;
        // errs() << blockCon[2].name << "\n";
        for (int i = blockCount; i > 0; i--){
            for (BasicBlock *Succ : successors(blockCon[i].block)) {
                for(int j = 0; j < blockCount; j++){
                    errs() << "Succ" << Succ<<"\n";
                    errs() << "block" << blockCon[j].block<<"\n";
                    errs() << "\n";
                    if (Succ == blockCon[j].block){
                        std::vector<StringRef> kill_U_UE;
                        std::set_union(blockCon[j].varKill.begin(), blockCon[j].varKill.end(), blockCon[j].ueVar.begin(), blockCon[j].ueVar.end(),std::back_inserter(kill_U_UE));
                        errs() << "print kill_U_UE" << "\n";
                        for (auto k: kill_U_UE){
                            errs() << k << ' ';
                        }
                        std::vector<StringRef> out_diff;
                        std::set_difference(blockCon[j].liveOut.begin(), blockCon[j].liveOut.end(), kill_U_UE.begin(), kill_U_UE.end(),std::back_inserter(out_diff));
                        std::set_union(blockCon[i].liveOut.begin(), blockCon[i].liveOut.end(), out_diff.begin(), out_diff.end(),std::back_inserter(blockCon[i].liveOut));
                    }
                }
                // errs() << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << "\n";
                // errs() << "liveOut:   " << blockCon[i].liveOut[0] << "\n";
            }
        }


        // for (auto &basic_block : F)
        // {
        //     for (BasicBlock *Pred : predecessors(&basic_block)) {

        //         errs() << basic_block << "\n";
        //         errs() << Pred->getName() << "\n";
        //         // for(int i=0 ; i < (sizeof(blockCon[blockCount].ueVar)/sizeof(blockCon[blockCount].ueVar[0])) ; i++){
        //         //     if(blockCon[blockCount].ueVar[i] != ""){
        //         //         errs() << "ueVar: " << blockCon[blockCount].ueVar[i] << "\n";
        //         //     }
        //         // }
        //         // // errs() << "ueVar: " << blockCon[blockCount].ueVar << "\n";
        //         // // errs() << "varKill: " << blockCon[blockCount].varKill << "\n";
        //         // blockCount--;
        //     }
        // }



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
