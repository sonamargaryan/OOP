#pragma once
#include <string>
#include <unordered_map>

class Symbol {
public:
    std::string name;
    int32_t     value;
    bool        isStatic;

    Symbol();
    Symbol(const std::string& n, int32_t v, bool s);
};

class SymbolTable {
public:
    void    declare(const std::string& name, int32_t initValue = 0, bool isStatic = false);
    void    setValue(const std::string& name, int32_t val);
    int32_t getValue(const std::string& name) const;
    bool    exists(const std::string& name) const;
    bool    isStatic(const std::string& name) const;

private:
    std::unordered_map<std::string, Symbol> table;
};
