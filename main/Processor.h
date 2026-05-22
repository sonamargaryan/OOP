#pragma once
#include "Memory.h"
#include <cstdint>
#include <string>
#include <vector>


class Processor {
public:
    explicit Processor(Memory& memory);

    void run(bool verbose = false);

    void printRegisters() const;

private:
    Memory&  memory;          
    uint32_t regs[16];        
    uint32_t IP;             
    bool     running;
    bool     verbose;

    std::vector<std::string> paramBuffer;

    // ── Fetch → Decode → Execute ───────────────────────────
    const TACInstruction& fetch();
    void                  execute(const TACInstruction& instr);

    // ── Variable access (Stack locals / Data globals) ──────
    int32_t  readVar(const std::string& name);
    void     writeVar(const std::string& name, int32_t value);
    int32_t  toValue(const std::string& s);
    bool     isNumber(const std::string& s) const;

    // ── Instruction handlers ───────────────────────────────
    void handleCopy(const TACInstruction& i);
    void handleArith(const TACInstruction& i);
    void handleCompare(const TACInstruction& i);
    void handleJump(const TACInstruction& i);
    void handleCall(const TACInstruction& i);
    void handleReturn(const TACInstruction& i);
};