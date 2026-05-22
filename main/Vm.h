#pragma once
#include "TACInstruction.h"
#include "Memory.h"
#include "Loader.h"
#include "Processor.h"

class VM {
public:
    VM();

    void load(const TACProgram& program);
    void run(bool verbose = false);

private:
    Memory    memory;
    Loader    loader;
    Processor processor;
};