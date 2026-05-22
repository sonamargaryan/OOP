#include "TACInstruction.h"

TACInstruction::TACInstruction(TACOp op,
                               const std::string& dest,
                               const std::string& src1,
                               const std::string& src2)
    : op(op), dest(dest), src1(src1), src2(src2) {}
    
std::string TACInstruction::opName(TACOp op) {
    switch (op) {
        case TACOp::ADD:        return "+";
        case TACOp::SUB:        return "-";
        case TACOp::MUL:        return "*";
        case TACOp::DIV:        return "/";
        case TACOp::NEG:        return "neg";
        case TACOp::LT:         return "<";
        case TACOp::GT:         return ">";
        case TACOp::LE:         return "<=";
        case TACOp::GE:         return ">=";
        case TACOp::EQ:         return "==";
        case TACOp::NE:         return "!=";
        case TACOp::COPY:       return "=";
        case TACOp::GOTO:       return "goto";
        case TACOp::IFFALSE:    return "iffalse";
        case TACOp::IFTRUE:     return "iftrue";
        case TACOp::LABEL:      return "label";
        case TACOp::FUNC_BEGIN: return "func_begin";
        case TACOp::FUNC_END:   return "func_end";
        case TACOp::PARAM:      return "param";
        case TACOp::CALL:       return "call";
        case TACOp::RETURN:     return "return";
        default:                return "?";
    }
}
std::string TACInstruction::toString() const {
    switch (op) {
        case TACOp::ADD:
        case TACOp::SUB:
        case TACOp::MUL:
        case TACOp::DIV:
        case TACOp::LT:
        case TACOp::GT:
        case TACOp::LE:
        case TACOp::GE:
        case TACOp::EQ:
        case TACOp::NE:
            return dest + " = " + src1 + " " + opName(op) + " " + src2;

        case TACOp::NEG:
            return dest + " = -" + src1;

        case TACOp::COPY:
            return dest + " = " + src1;

        case TACOp::GOTO:
            return "goto " + dest;

        case TACOp::IFFALSE:
            return "if " + src1 + " == 0  goto " + dest;

        case TACOp::IFTRUE:
            return "if " + src1 + " != 0  goto " + dest;

        case TACOp::LABEL:
            return dest + ":";

        case TACOp::FUNC_BEGIN:
            return "── func " + dest + " ──────────────";

        case TACOp::FUNC_END:
            return "── end  " + dest + " ──────────────";

        case TACOp::PARAM:
            return "param " + src1;

        case TACOp::CALL:
            return dest.empty()
                ? "call " + src1 + ", " + src2
                : dest + " = call " + src1 + ", " + src2;

        case TACOp::RETURN:
            return src1.empty() ? "return" : "return " + src1;

        default:
            return "???";
    }
}