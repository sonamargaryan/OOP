
#include "Interpreter.h"
#include <iostream>
#include <sstream>
#include <memory>

Interpreter::Interpreter() : currentInput(nullptr) {
    calculator = std::make_unique<Calculator>(symbolTable);
}

void Interpreter::setInput(const std::string& expression) {
    auto inputStream = std::make_unique<std::istringstream>(expression);
    currentInput = inputStream.get();
    lexer = std::make_unique<Lexer>(*inputStream);
    parser = std::make_unique<Parser>(*lexer, symbolTable);
    ownedStreams.push_back(std::move(inputStream));
}

void Interpreter::setInput(std::istream& is) {
    currentInput = &is;
    lexer = std::make_unique<Lexer>(is);
    parser = std::make_unique<Parser>(*lexer, symbolTable);
}

double Interpreter::evaluate() {
    if (!parser) {
        throw std::runtime_error("No input set");
    }
    
    auto ast = parser->parse();
    return calculator->calculate(std::move(ast));
}

void Interpreter::setVariable(const std::string& name, double value) {
    symbolTable.setValue(name, value);
}

bool Interpreter::getVariable(const std::string& name, double& value) {
    return symbolTable.getValue(name, value);
}

void Interpreter::reset() {
    lexer.reset();
    parser.reset();
    ownedStreams.clear();
}
