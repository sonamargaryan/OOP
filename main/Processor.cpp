#include "Processor.h"
#include <iostream>
#include <stdexcept>
#include <cctype>

Processor::Processor(Memory& memory)
    : memory(memory), IP(0), running(false), verbose(false)
{
    for (int i = 0; i < 16; i++) regs[i] = 0;
}

bool Processor::isNumber(const std::string& s) const {
    if (s.empty()) return false;
    int start = (s[0] == '-') ? 1 : 0;
    for (int i = start; i < (int)s.size(); i++)
        if (!isdigit(s[i])) return false;
    return s.size() > (size_t)start;
}

int32_t Processor::toValue(const std::string& s) {
    if (isNumber(s)) return std::stoi(s);
    return readVar(s);
}

int32_t Processor::readVar(const std::string& name) {
    if (!memory.stackEmpty()) {
        auto& locals = memory.topFrame().locals;
        auto it = locals.find(name);
        if (it != locals.end()) return it->second;
    }
    return memory.loadData(name);
}

void Processor::writeVar(const std::string& name, int32_t value) {
    if (!memory.stackEmpty()) {
        memory.topFrame().locals[name] = value;
    }
    if (memory.dataExists(name))
        memory.storeData(name, value);
}

void Processor::run(bool verb) {
    verbose = verb;
    running = true;

    IP = memory.getFuncAddress("main");

    memory.pushFrame(CallFrame("main", memory.codeSize()));
    IP++;  

    if (verbose) {
        std::cout << "\n═══ PROCESSOR EXECUTION ═══\n";
        std::cout << "  Registers: " << 16 << " x 32-bit\n";
        std::cout << "  Starting at IP=" << IP << "\n\n";
    }

    while (running && IP < memory.codeSize()) {
        const TACInstruction& instr = fetch();

        if (verbose)
            std::cout << "  [IP=" << IP << "] " << instr.toString() << "\n";

        execute(instr);
    }
}

const TACInstruction& Processor::fetch() {
    return memory.fetchInstruction(IP);
}

void Processor::execute(const TACInstruction& instr) {
    switch (instr.op) {

        case TACOp::FUNC_BEGIN:
        case TACOp::LABEL:
            IP++;
            break;

        case TACOp::FUNC_END:
            if (!memory.stackEmpty()) {
                uint32_t retIP = memory.topFrame().returnIP;
                memory.popFrame();
                if (memory.stackEmpty()) {
                    running = false;
                } else {
                    IP = retIP;
                }
            } else {
                running = false;
            }
            break;

        case TACOp::COPY:
            handleCopy(instr);
            break;

        case TACOp::ADD:
        case TACOp::SUB:
        case TACOp::MUL:
        case TACOp::DIV:
        case TACOp::NEG:
            handleArith(instr);
            break;

        case TACOp::LT:
        case TACOp::GT:
        case TACOp::LE:
        case TACOp::GE:
        case TACOp::EQ:
        case TACOp::NE:
            handleCompare(instr);
            break;

        case TACOp::GOTO:
        case TACOp::IFFALSE:
        case TACOp::IFTRUE:
            handleJump(instr);
            break;

        case TACOp::PARAM:
            paramBuffer.push_back(instr.src1);
            IP++;
            break;

        case TACOp::CALL:
            handleCall(instr);
            break;

        case TACOp::RETURN:
            handleReturn(instr);
            break;

        default:
            IP++;
            break;
    }
}

void Processor::handleCopy(const TACInstruction& i) {
    if (!i.src1.empty() && i.src1.size() > 4 &&
        i.src1.substr(0,4) == "arg_") {
        IP++; return;
    }
    writeVar(i.dest, toValue(i.src1));
    IP++;
}

void Processor::handleArith(const TACInstruction& i) {
    int32_t result = 0;

    if (i.op == TACOp::NEG) {
        result = -toValue(i.src1);
    } else {
        int32_t a = toValue(i.src1);
        int32_t b = toValue(i.src2);
        switch (i.op) {
            case TACOp::ADD: result = a + b; break;
            case TACOp::SUB: result = a - b; break;
            case TACOp::MUL: result = a * b; break;
            case TACOp::DIV:
                if (b == 0) throw std::runtime_error("Processor: division by zero");
                result = a / b; break;
            default: break;
        }
    }

    writeVar(i.dest, result);
    IP++;
}

void Processor::handleCompare(const TACInstruction& i) {
    int32_t a = toValue(i.src1);
    int32_t b = toValue(i.src2);
    int32_t result = 0;

    switch (i.op) {
        case TACOp::LT: result = (a <  b) ? 1 : 0; break;
        case TACOp::GT: result = (a >  b) ? 1 : 0; break;
        case TACOp::LE: result = (a <= b) ? 1 : 0; break;
        case TACOp::GE: result = (a >= b) ? 1 : 0; break;
        case TACOp::EQ: result = (a == b) ? 1 : 0; break;
        case TACOp::NE: result = (a != b) ? 1 : 0; break;
        default: break;
    }

    writeVar(i.dest, result);
    IP++;
}

void Processor::handleJump(const TACInstruction& i) {
    if (i.op == TACOp::GOTO) {
        IP = memory.getLabelAddress(i.dest);
        return;
    }
    if (i.op == TACOp::IFFALSE) {
        if (toValue(i.src1) == 0)
            IP = memory.getLabelAddress(i.dest);
        else
            IP++;
        return;
    }
    if (i.op == TACOp::IFTRUE) {
        if (toValue(i.src1) != 0)
            IP = memory.getLabelAddress(i.dest);
        else
            IP++;
        return;
    }
}

void Processor::handleCall(const TACInstruction& i) {
    uint32_t funcIP = memory.getFuncAddress(i.src1);

    CallFrame frame(i.src1, IP + 1);

    uint32_t scanIP = funcIP + 1;
    int argIdx = 0;
    while (scanIP < memory.codeSize()) {
        const TACInstruction& cop = memory.fetchInstruction(scanIP);
        if (cop.op == TACOp::COPY &&
            !cop.src1.empty() &&
            cop.src1.size() > 4 &&
            cop.src1.substr(0,4) == "arg_") {
            if (argIdx < (int)paramBuffer.size())
                frame.locals[cop.dest] = toValue(paramBuffer[argIdx]);
            argIdx++;
            scanIP++;
        } else {
            break;
        }
    }

    paramBuffer.clear();

    if (!i.dest.empty())
        frame.locals["__call_dest__"] = 0;

    memory.pushFrame(frame);

    if (verbose)
        std::cout << "  → CALL " << i.src1
                  << " (stack depth=" << memory.stackDepth() << ")\n";

    IP = funcIP;
}

void Processor::handleReturn(const TACInstruction& i) {
    int32_t retVal = 0;
    if (!i.src1.empty())
        retVal = toValue(i.src1);

    uint32_t retIP = memory.codeSize();
    if (!memory.stackEmpty())
        retIP = memory.topFrame().returnIP;

    memory.popFrame();

    if (memory.stackEmpty()) {
        std::cout << "\n═══ VM RESULT ═══\n";
        std::cout << "  Return value: " << retVal << "\n";
        printRegisters();
        running = false;
    } else {
        if (retIP > 0 && retIP <= memory.codeSize()) {
            const TACInstruction& callInstr = memory.fetchInstruction(retIP - 1);
            if (callInstr.op == TACOp::CALL && !callInstr.dest.empty())
                writeVar(callInstr.dest, retVal);
        }
        IP = retIP;
    }
}

void Processor::printRegisters() const {
    std::cout << "\n  Registers (non-zero):\n";
    bool any = false;
    for (int i = 0; i < 16; i++) {
        if (regs[i] != 0) {
            std::cout << "    r" << i << " = " << regs[i] << "\n";
            any = true;
        }
    }
    if (!any) std::cout << "    (all zero)\n";
}