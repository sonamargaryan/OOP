#include "Optimizer.h"
#include <unordered_map>
#include <cctype>
#include <stdexcept>

Optimizer::Optimizer() {}
TACProgram Optimizer::optimize(const TACProgram& input) {
    TACProgram code = input;
    code = constantFolding(code);   
    code = copyPropagation(code);   
    code = deadCodeElim(code);      
    return code;
}

bool Optimizer::isNumber(const std::string& s) const {
    if (s.empty()) return false;
    int start = (s[0] == '-') ? 1 : 0;
    for (int i = start; i < (int)s.size(); i++)
        if (!isdigit(s[i])) return false;
    return s.size() > (size_t)start;
}

int Optimizer::toNumber(const std::string& s) const {
    return std::stoi(s);
}

bool Optimizer::isArithmetic(TACOp op) const {
    return op == TACOp::ADD || op == TACOp::SUB ||
           op == TACOp::MUL || op == TACOp::DIV;
}

bool Optimizer::isComparison(TACOp op) const {
    return op == TACOp::LT || op == TACOp::GT ||
           op == TACOp::LE || op == TACOp::GE ||
           op == TACOp::EQ || op == TACOp::NE;
}

int Optimizer::applyOp(TACOp op, int a, int b) const {
    switch (op) {
        case TACOp::ADD: return a + b;
        case TACOp::SUB: return a - b;
        case TACOp::MUL: return a * b;
        case TACOp::DIV:
            if (b == 0) throw std::runtime_error("Optimizer: division by zero");
            return a / b;
        case TACOp::LT:  return a <  b ? 1 : 0;
        case TACOp::GT:  return a >  b ? 1 : 0;
        case TACOp::LE:  return a <= b ? 1 : 0;
        case TACOp::GE:  return a >= b ? 1 : 0;
        case TACOp::EQ:  return a == b ? 1 : 0;
        case TACOp::NE:  return a != b ? 1 : 0;
        default: return 0;
    }
}

TACProgram Optimizer::constantFolding(const TACProgram& code) {
    TACProgram result;

    for (const TACInstruction& instr : code) {
        if ((isArithmetic(instr.op) || isComparison(instr.op)) &&
             isNumber(instr.src1) && isNumber(instr.src2)) {

            int a   = toNumber(instr.src1);
            int b   = toNumber(instr.src2);
            int res = applyOp(instr.op, a, b);

            result.push_back({TACOp::COPY, instr.dest, std::to_string(res)});
            continue;
        }

        if (instr.op == TACOp::NEG && isNumber(instr.src1)) {
            int val = -toNumber(instr.src1);
            result.push_back({TACOp::COPY, instr.dest, std::to_string(val)});
            continue;
        }

        result.push_back(instr);
    }

    return result;
}
TACProgram Optimizer::copyPropagation(const TACProgram& code) {
    std::unordered_map<std::string, std::string> copies;

    TACProgram result;

    for (TACInstruction instr : code) {

        if (!instr.src1.empty()) {
            auto it = copies.find(instr.src1);
            if (it != copies.end())
                instr.src1 = it->second;
        }

        if (!instr.src2.empty()) {
            auto it = copies.find(instr.src2);
            if (it != copies.end())
                instr.src2 = it->second;
        }

        if (instr.op == TACOp::COPY && !instr.dest.empty()) {
            if (!instr.dest.empty() && instr.dest[0] == 't' &&
                instr.dest.size() > 1 && isdigit(instr.dest[1])) {
                copies[instr.dest] = instr.src1;
            } else {
                for (auto it = copies.begin(); it != copies.end(); ) {
                    if (it->second == instr.dest)
                        it = copies.erase(it);
                    else
                        ++it;
                }
            }
        }

        if (instr.op == TACOp::LABEL   ||
            instr.op == TACOp::GOTO    ||
            instr.op == TACOp::IFFALSE ||
            instr.op == TACOp::IFTRUE) {
            copies.clear();
        }

        result.push_back(instr);
    }

    return result;
}

TACProgram Optimizer::deadCodeElim(const TACProgram& code) {
    TACProgram result;

    for (const TACInstruction& instr : code) {

        if (instr.op == TACOp::ADD) {
            if (instr.src2 == "0" && instr.dest == instr.src1) continue;
            if (instr.src1 == "0" && instr.dest == instr.src2) continue;
        }

        if (instr.op == TACOp::SUB) {
            if (instr.src2 == "0" && instr.dest == instr.src1) continue;
        }

        if (instr.op == TACOp::MUL) {
            if (instr.src2 == "1" && instr.dest == instr.src1) continue;
            if (instr.src1 == "1" && instr.dest == instr.src2) continue;
        }

        if (instr.op == TACOp::DIV) {
            if (instr.src2 == "1" && instr.dest == instr.src1) continue;
        }

        result.push_back(instr);
    }

    return result;
}