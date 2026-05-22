#include "CodeGenerator.h"
#include <stdexcept>
#include <cctype>
#include <set>

CodeGenerator::CodeGenerator(std::ostream& output)
    : out(output), frameSize(0), nextOffset(0) {}

void CodeGenerator::emit(const std::string& instr) {
    out << "    " << instr << "\n";
}

void CodeGenerator::emitLabel(const std::string& label) {
    out << label << ":\n";
}

bool CodeGenerator::hasVar(const std::string& name) const {
    return varOffset.count(name) > 0;
}

void CodeGenerator::allocVar(const std::string& name) {
    if (!hasVar(name)) {
        nextOffset -= 4;
        varOffset[name] = nextOffset;
    }
}

int CodeGenerator::getOffset(const std::string& name) {
    auto it = varOffset.find(name);
    if (it == varOffset.end())
        throw std::runtime_error("CodeGen: unknown var '" + name + "'");
    return it->second;
}

void CodeGenerator::load(const std::string& src, const std::string& reg) {
    if (src.empty()) return;
    bool isLit = !src.empty() && (isdigit(src[0]) || src[0] == '-');
    if (isLit)
        emit("li   " + reg + ", " + src);
    else
        emit("lw   " + reg + ", " + std::to_string(getOffset(src)) + "(fp)");
}

void CodeGenerator::store(const std::string& dest, const std::string& reg) {
    emit("sw   " + reg + ", " + std::to_string(getOffset(dest)) + "(fp)");
}

void CodeGenerator::buildFrame(const TACProgram& func,
                                const std::vector<std::string>& params) {
    varOffset.clear();
    nextOffset = -8;  

    for (const auto& p : params)
        allocVar(p);

    for (const auto& instr : func) {
        if (!instr.dest.empty() &&
            instr.op != TACOp::GOTO   &&
            instr.op != TACOp::LABEL  &&
            instr.op != TACOp::IFFALSE &&
            instr.op != TACOp::IFTRUE  &&
            instr.op != TACOp::FUNC_BEGIN &&
            instr.op != TACOp::FUNC_END)
            allocVar(instr.dest);

        if (!instr.src1.empty() && !isdigit(instr.src1[0]) && instr.src1[0] != '-')
            allocVar(instr.src1);
        if (!instr.src2.empty() && !isdigit(instr.src2[0]) && instr.src2[0] != '-')
            allocVar(instr.src2);
    }

    int slots = (-nextOffset / 4) + 1;  
    frameSize = ((slots * 4 + 8) + 15) & ~15;
}

void CodeGenerator::emitPrologue() {
    emit("addi sp, sp, -" + std::to_string(frameSize));
    emit("sw   ra, " + std::to_string(frameSize - 4) + "(sp)");
    emit("sw   fp, " + std::to_string(frameSize - 8) + "(sp)");
    emit("addi fp, sp, " + std::to_string(frameSize));
}

void CodeGenerator::emitEpilogue() {
    emitLabel(currentFunc + "_end");
    emit("lw   ra, " + std::to_string(frameSize - 4) + "(sp)");
    emit("lw   fp, " + std::to_string(frameSize - 8) + "(sp)");
    emit("addi sp, sp, " + std::to_string(frameSize));
    emit("ret");
}

void CodeGenerator::generate(const TACProgram& program) {
    out << "    .text\n";
    out << "    .globl main\n\n";

    TACProgram funcCode;
    bool inFunc = false;

    for (const auto& instr : program) {
        if (instr.op == TACOp::FUNC_BEGIN) {
            inFunc = true;
            funcCode.clear();
            funcCode.push_back(instr);
        } else if (instr.op == TACOp::FUNC_END) {
            funcCode.push_back(instr);
            genFunction(funcCode);
            inFunc = false;
        } else if (inFunc) {
            funcCode.push_back(instr);
        }
    }
}

void CodeGenerator::genFunction(const TACProgram& func) {
    currentFunc = func.front().dest;

    std::vector<std::string> params;
    for (const auto& instr : func) {
        if (instr.op == TACOp::COPY &&
            instr.src1.size() > 4 &&
            instr.src1.substr(0, 4) == "arg_")
            params.push_back(instr.dest);
    }

    buildFrame(func, params);

    out << "\n";
    emitLabel(currentFunc);
    emitPrologue();

    static const char* argRegs[] = {"a0","a1","a2","a3","a4","a5","a6","a7"};
    for (int i = 0; i < (int)params.size(); i++)
        emit("sw   " + std::string(argRegs[i]) + ", " +
             std::to_string(getOffset(params[i])) + "(fp)");

    for (const auto& instr : func) {
        if (instr.op == TACOp::FUNC_BEGIN ||
            instr.op == TACOp::FUNC_END) continue;
        if (instr.op == TACOp::COPY &&
            !instr.src1.empty() &&
            instr.src1.size() > 4 &&
            instr.src1.substr(0,4) == "arg_") continue;

        genInstruction(instr);
    }

    emitEpilogue();
}

void CodeGenerator::genInstruction(const TACInstruction& instr) {
    switch (instr.op) {
        case TACOp::COPY:
            load(instr.src1, "t0");
            store(instr.dest, "t0");
            break;

        case TACOp::ADD:
        case TACOp::SUB:
        case TACOp::MUL:
        case TACOp::DIV: {
            load(instr.src1, "t0");
            load(instr.src2, "t1");
            std::string op;
            if (instr.op == TACOp::ADD) op = "add  t0, t0, t1";
            if (instr.op == TACOp::SUB) op = "sub  t0, t0, t1";
            if (instr.op == TACOp::MUL) op = "mul  t0, t0, t1";
            if (instr.op == TACOp::DIV) op = "div  t0, t0, t1";
            emit(op);
            store(instr.dest, "t0");
            break;
        }

        case TACOp::NEG:
            load(instr.src1, "t0");
            emit("neg  t0, t0");
            store(instr.dest, "t0");
            break;

        case TACOp::LT:
        case TACOp::GT:
        case TACOp::LE:
        case TACOp::GE:
        case TACOp::EQ:
        case TACOp::NE: {
            load(instr.src1, "t0");
            load(instr.src2, "t1");
            switch (instr.op) {
                case TACOp::LT: emit("slt  t0, t0, t1"); break;
                case TACOp::GT: emit("slt  t0, t1, t0"); break;
                case TACOp::LE: emit("slt  t0, t1, t0"); emit("xori t0, t0, 1"); break;
                case TACOp::GE: emit("slt  t0, t0, t1"); emit("xori t0, t0, 1"); break;
                case TACOp::EQ: emit("xor  t0, t0, t1"); emit("seqz t0, t0"); break;
                case TACOp::NE: emit("xor  t0, t0, t1"); emit("snez t0, t0"); break;
                default: break;
            }
            store(instr.dest, "t0");
            break;
        }

        case TACOp::LABEL:
            emitLabel(instr.dest);
            break;

        case TACOp::GOTO:
            emit("j    " + instr.dest);
            break;

        case TACOp::IFFALSE:
            load(instr.src1, "t0");
            emit("beq  t0, zero, " + instr.dest);
            break;

        case TACOp::IFTRUE:
            load(instr.src1, "t0");
            emit("bne  t0, zero, " + instr.dest);
            break;

        case TACOp::RETURN:
            if (!instr.src1.empty()) {
                load(instr.src1, "a0");
            }
            emit("j    " + currentFunc + "_end");
            break;

        case TACOp::PARAM:
            emit("# param " + instr.src1);
            break;

        case TACOp::CALL: {
            emit("call " + instr.src1);
            if (!instr.dest.empty())
                store(instr.dest, "a0");
            break;
        }

        default:
            break;
    }
}