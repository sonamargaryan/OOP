#include "SymbolTable.h"
#include <stdexcept>


Symbol::Symbol()
    : name(""), value(0), isStatic(false) {}

Symbol::Symbol(const std::string& n, int32_t v, bool s)
    : name(n), value(v), isStatic(s) {}

void SymbolTable::declare(const std::string& name, int32_t initValue, bool isStatic) {
    if (table.count(name))
        throw std::runtime_error("Variable already declared: " + name);
    table[name] = Symbol(name, initValue, isStatic);
}

void SymbolTable::setValue(const std::string& name, int32_t val) {
    if (!table.count(name))
        throw std::runtime_error("Undeclared variable: " + name);
    table[name].value = val;
}

int32_t SymbolTable::getValue(const std::string& name) const {
    auto it = table.find(name);
    if (it == table.end())
        throw std::runtime_error("Undeclared variable: " + name);
    return it->second.value;
}

bool SymbolTable::exists(const std::string& name) const {
    return table.count(name) > 0;
}

bool SymbolTable::isStatic(const std::string& name) const {
    auto it = table.find(name);
    if (it == table.end()) return false;
    return it->second.isStatic;
}
