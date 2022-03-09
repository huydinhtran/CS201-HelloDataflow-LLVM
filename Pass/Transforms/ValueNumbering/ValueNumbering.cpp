/*
CS201 Compiler Construction - Project 3
Group members: Huy Dinh Tran, Rajat Jain
*/

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/IR/CFG.h"
#include <set>
#include <iostream>
#include <vector>
#include <iterator>
#include <iomanip>
#include <algorithm>

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
        string func_name = "test1";
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
                        loadCount++;
                    }
                }
                if (inst.isBinaryOp())
                {
                    if (inst.getOpcode() == Instruction::Add)
                    {
                        blockCon[blockCount].ueVar.insert(blockCon[blockCount].ueVar.end(),hTable[loadCount-2].name);
                        blockCon[blockCount].ueVar.insert(blockCon[blockCount].ueVar.end(),hTable[loadCount-1].name);
                        ueVarCount += 2;
                        afterOp = true;
                    }
                }
                if (inst.getOpcode() == Instruction::Store)
                {
                    if(afterOp == true){
                        hTable[tableCount].var = var2;
                        hTable[tableCount].name = var2Name;
                        blockCon[blockCount].varKill.insert(blockCon[blockCount].varKill.end(),hTable[tableCount].name);
                        varKillCount++;
                        tableCount++;
                        afterOp = false;
                    }else{
                        blockCon[blockCount].ueVar.insert(blockCon[blockCount].ueVar.end(),hTable[loadCount-1].name);
                        blockCon[blockCount].varKill.insert(blockCon[blockCount].varKill.end(),hTable[loadCount-2].name);
                        ueVarCount++;
                        varKillCount++;
                        tableCount++;
                    }
                }
            }
            sort(blockCon[blockCount].ueVar);
            blockCon[blockCount].ueVar.erase( unique( blockCon[blockCount].ueVar.begin(), blockCon[blockCount].ueVar.end() ), blockCon[blockCount].ueVar.end() );
            sort(blockCon[blockCount].varKill);
            blockCon[blockCount].varKill.erase( unique( blockCon[blockCount].varKill.begin(), blockCon[blockCount].varKill.end() ), blockCon[blockCount].varKill.end() );
        }


//////////////////////////////Implementing LiveOut//////////////////////////////

        for (int i = blockCount-1; i >= 0; i--){
            for (BasicBlock *Succ : successors(blockCon[i].block)) {
                for(int j = 0; j < blockCount+1; j++){
                    if (Succ == blockCon[j].block){
                        std::vector<StringRef> out_diff_kill;
                        std::set_difference(blockCon[j].liveOut.begin(), blockCon[j].liveOut.end(), blockCon[j].varKill.begin(), blockCon[j].varKill.end(),std::back_inserter(out_diff_kill));
                        std::vector<StringRef> U_AE;
                        std::set_union(out_diff_kill.begin(), out_diff_kill.end(), blockCon[j].ueVar.begin(), blockCon[j].ueVar.end(),std::back_inserter(U_AE));
                        std::set_union(blockCon[i].liveOut.begin(), blockCon[i].liveOut.end(), U_AE.begin(), U_AE.end(),std::back_inserter(blockCon[i].liveOut));
                    }
                }
            }
            sort(blockCon[i].liveOut);
            blockCon[i].liveOut.erase( unique( blockCon[i].liveOut.begin(), blockCon[i].liveOut.end() ), blockCon[i].liveOut.end() );
        }


//////////////////////////////Printing Output//////////////////////////////

        for(int i = 0; i < blockCount+1; i++){
            errs()<<"-----"<< blockCon[i].name << "-----" << "\n";
            errs()<<"UEVAR: ";
            for(auto j: blockCon[i].ueVar){
                errs()<< j << " ";
            }
            errs()<< "\n";
            errs()<<"VARKILL: ";
            for(auto k: blockCon[i].varKill){
                errs()<< k << " ";
            }
            errs()<< "\n";
            errs()<<"VARKILL: ";
            for(auto l: blockCon[i].liveOut){
                errs()<< l << " ";
            }
            errs()<< "\n";
        }



        for (auto &basic_block : F)
        {
            for (auto &inst : basic_block)
            {
                if (inst.isBinaryOp())
                {
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
