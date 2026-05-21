#pragma once
#include "TACInstruction.h"
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

class CodeGenerator {
public:
    explicit CodeGenerator(std::ostream& output);

    void generate(const TACProgram& program);

private:
    std::ostream& out;

    std::string                          currentFunc;
    std::unordered_map<std::string, int> varOffset;  
    int                                  frameSize;
    int                                  nextOffset;

    void genFunction(const TACProgram& func);

    void buildFrame(const TACProgram& func,
                    const std::vector<std::string>& params);

    void genInstruction(const TACInstruction& instr);

    void emitPrologue();
    void emitEpilogue();

    void        emit(const std::string& instr);
    void        emitLabel(const std::string& label);
    void        allocVar(const std::string& name);
    int         getOffset(const std::string& name);
    bool        hasVar(const std::string& name) const;

    void        load(const std::string& src, const std::string& reg = "t0");
    void        store(const std::string& dest, const std::string& reg = "t0");

    std::string invertedBranch(TACOp op);
};