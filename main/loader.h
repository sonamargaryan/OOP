#pragma once
#include "Memory.h"
#include "TACInstruction.h"

class Loader {
public:
    Loader();

    void load(const TACProgram& program, Memory& memory);

    void printLoadInfo(const Memory& memory);

private:
    void loadCode(const TACProgram& program, Memory& memory);

    void initData(const TACProgram& program, Memory& memory);

    void buildTables(const TACProgram& program, Memory& memory);

    bool isGlobalVar(const std::string& name) const;
};