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

    void     storeInstruction(const TACInstruction& instr);
    const TACInstruction& fetchInstruction(uint32_t address) const;
    uint32_t codeSize() const;

    void    storeData(const std::string& name, int32_t value);
    int32_t loadData(const std::string& name) const;
    bool    dataExists(const std::string& name) const;

    void       pushFrame(const CallFrame& frame);
    void       popFrame();
    CallFrame& topFrame();
    bool       stackEmpty() const;
    int        stackDepth() const;

    void     addLabel(const std::string& name, uint32_t address);
    void     addFunction(const std::string& name, uint32_t address);
    uint32_t getLabelAddress(const std::string& name) const;
    uint32_t getFuncAddress(const std::string& name) const;
    bool     hasFunction(const std::string& name) const;

    void printLayout() const;

private:
    std::vector<TACInstruction> codeSection;

    std::unordered_map<std::string, int32_t> dataSection;

    std::stack<CallFrame> stackSection;

    std::unordered_map<std::string, uint32_t> labelTable;
    std::unordered_map<std::string, uint32_t> funcTable;
};