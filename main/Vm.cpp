#include "VM.h"
#include <iostream>

VM::VM() : processor(memory) {}

void VM::load(const TACProgram& program) {
    loader.load(program, memory);
    loader.printLoadInfo(memory);
}

void VM::run(bool verbose) {
    processor.run(verbose);
    memory.printLayout(); 
}