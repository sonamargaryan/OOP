#include "Memory.h"
#include <iostream>

//  CALL FRAME
CallFrame::CallFrame(const std::string& name, uint32_t retIP)
    : funcName(name), returnIP(retIP) {}

//  MEMORY Constructor
Memory::Memory() {}

//  CODE SECTION
void Memory::storeInstruction(const TACInstruction& instr) {
    codeSection.push_back(instr);
}

const TACInstruction& Memory::fetchInstruction(uint32_t address) const {
    if (address >= codeSection.size())
        throw std::runtime_error("Memory: code address out of bounds: " +
                                 std::to_string(address));
    return codeSection[address];
}

uint32_t Memory::codeSize() const {
    return (uint32_t)codeSection.size();
}

//  DATA SECTION
void Memory::storeData(const std::string& name, int32_t value) {
    dataSection[name] = value;
}

int32_t Memory::loadData(const std::string& name) const {
    auto it = dataSection.find(name);
    if (it == dataSection.end()) return 0;
    return it->second;
}

bool Memory::dataExists(const std::string& name) const {
    return dataSection.count(name) > 0;
}

//  STACK SECTION
void Memory::pushFrame(const CallFrame& frame) {
    stackSection.push(frame);
}

void Memory::popFrame() {
    if (stackSection.empty())
        throw std::runtime_error("Memory: stack underflow");
    stackSection.pop();
}

CallFrame& Memory::topFrame() {
    if (stackSection.empty())
        throw std::runtime_error("Memory: stack is empty");
    return stackSection.top();
}

bool Memory::stackEmpty() const {
    return stackSection.empty();
}

int Memory::stackDepth() const {
    return (int)stackSection.size();
}

//  LABEL / FUNCTION TABLES
void Memory::addLabel(const std::string& name, uint32_t address) {
    labelTable[name] = address;
}

void Memory::addFunction(const std::string& name, uint32_t address) {
    funcTable[name] = address;
}

uint32_t Memory::getLabelAddress(const std::string& name) const {
    auto it = labelTable.find(name);
    if (it == labelTable.end())
        throw std::runtime_error("Memory: unknown label '" + name + "'");
    return it->second;
}

uint32_t Memory::getFuncAddress(const std::string& name) const {
    auto it = funcTable.find(name);
    if (it == funcTable.end())
        throw std::runtime_error("Memory: unknown function '" + name + "'");
    return it->second;
}

bool Memory::hasFunction(const std::string& name) const {
    return funcTable.count(name) > 0;
}

//  DEBUG — Memory layout print
void Memory::printLayout() const {
    std::cout << "\n═══ MEMORY LAYOUT ═══\n";

    std::cout << "┌─────────────────────────────┐\n";
    std::cout << "│  CODE SECTION (" << codeSection.size() << " instructions)\n";
    std::cout << "├─────────────────────────────┤\n";

    std::cout << "│  DATA SECTION (" << dataSection.size() << " variables)\n";
    for (auto& kv : dataSection)
        std::cout << "│    " << kv.first << " = " << kv.second << "\n";

    std::cout << "├─────────────────────────────┤\n";
    std::cout << "│  STACK SECTION (depth=" << stackSection.size() << ")\n";
    std::cout << "└─────────────────────────────┘\n";
}