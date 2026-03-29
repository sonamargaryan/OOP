#include <iostream>
#include <string>
#include "Interpreter.h"

int main() {
    Interpreter interpreter;

    std::cout << "=== Expression Interpreter ===\n";
    std::cout << "Examples:\n";
    std::cout << "  x = 5 + 3\n";
    std::cout << "  y = x * 2\n";
    std::cout << "  y\n";
    std::cout << "Type 'exit' to quit\n\n";

    std::string input;

    while (true) {
        std::cout << ">>> ";
        std::getline(std::cin, input);

        if (input == "exit" || input == "quit") {
            break;
        }

        if (input.empty()) {
            continue;
        }

        try {
            interpreter.setInput(input);
            double result = interpreter.evaluate();

            std::cout << result << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}
