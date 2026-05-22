#pragma once
#include "TACInstruction.h"

class Optimizer {
public:
    Optimizer();

    TACProgram optimize(const TACProgram& input);

private:
    TACProgram constantFolding(const TACProgram& code);

    TACProgram copyPropagation(const TACProgram& code);

    TACProgram deadCodeElim(const TACProgram& code);

    bool        isNumber(const std::string& s) const;
    int         toNumber(const std::string& s) const;
    int         applyOp(TACOp op, int a, int b) const;
    bool        isArithmetic(TACOp op) const;
    bool        isComparison(TACOp op) const;
};