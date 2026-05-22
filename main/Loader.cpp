#include "Loader.h"
#include <iostream>
#include <cctype>

Loader::Loader() {}

void Loader::load(const TACProgram& program, Memory& memory) {
    std::cout << "\n═══ LOADER ═══\n";
    std::cout << "  Loading " << program.size() << " instructions...\n";

    loadCode(program, memory);

    buildTables(program, memory);

    initData(program, memory);

    std::cout << " Code section:  " << memory.codeSize() << " instructions\n";
    std::cout << " Loader done\n";
}

void Loader::loadCode(const TACProgram& program, Memory& memory) {
    for (const TACInstruction& instr : program)
        memory.storeInstruction(instr);
}

void Loader::buildTables(const TACProgram& program, Memory& memory) {
    for (uint32_t i = 0; i < (uint32_t)program.size(); i++) {
        const TACInstruction& instr = program[i];

        if (instr.op == TACOp::LABEL)
            memory.addLabel(instr.dest, i);

        if (instr.op == TACOp::FUNC_BEGIN)
            memory.addFunction(instr.dest, i);
    }
}

void Loader::initData(const TACProgram& program, Memory& memory) {
    for (const TACInstruction& instr : program) {
        if (!instr.dest.empty() &&
            instr.op != TACOp::GOTO   &&
            instr.op != TACOp::LABEL  &&
            instr.op != TACOp::IFFALSE &&
            instr.op != TACOp::IFTRUE  &&
            instr.op != TACOp::FUNC_BEGIN &&
            instr.op != TACOp::FUNC_END) {
            if (isGlobalVar(instr.dest) && !memory.dataExists(instr.dest))
                memory.storeData(instr.dest, 0);
        }
    }
}

bool Loader::isGlobalVar(const std::string& name) const {
    if (name.empty()) return false;
    if (name[0] == 't' && name.size() > 1 && isdigit(name[1]))
        return false;
    if (name.size() > 4 && name.substr(0,4) == "arg_")
        return false;
    return true;
}
void Loader::printLoadInfo(const Memory& memory) {
    memory.printLayout();
}