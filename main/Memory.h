#pragma once
#include "TACInstruction.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <stdexcept>

class CallFrame {
public:
    std::string                              funcName;
    uint32_t                                 returnIP;
    std::unordered_map<std::string, int32_t> locals;

    CallFrame(const std::string& name, uint32_t retIP);
};

class Memory {
public:
    Memory();

    // ── CODE SECTION ───────────────────────────────────────
    void     storeInstruction(const TACInstruction& instr);
    const TACInstruction& fetchInstruction(uint32_t address) const;
    uint32_t codeSize() const;

    // ── DATA SECTION ───────────────────────────────────────
    void    storeData(const std::string& name, int32_t value);
    int32_t loadData(const std::string& name) const;
    bool    dataExists(const std::string& name) const;

    // ── STACK SECTION ──────────────────────────────────────
    void       pushFrame(const CallFrame& frame);
    void       popFrame();
    CallFrame& topFrame();
    bool       stackEmpty() const;
    int        stackDepth() const;

    // ── Label / Function tables ────────────────────────────
    void     addLabel(const std::string& name, uint32_t address);
    void     addFunction(const std::string& name, uint32_t address);
    uint32_t getLabelAddress(const std::string& name) const;
    uint32_t getFuncAddress(const std::string& name) const;
    bool     hasFunction(const std::string& name) const;

    // ── Debug ──────────────────────────────────────────────
    void printLayout() const;

private:
    // Code section
    std::vector<TACInstruction> codeSection;

    // Data section
    std::unordered_map<std::string, int32_t> dataSection;

    // Stack section
    std::stack<CallFrame> stackSection;

    // Tables
    std::unordered_map<std::string, uint32_t> labelTable;
    std::unordered_map<std::string, uint32_t> funcTable;
};