#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "ProgramParser.h"
#include "IRGenerator.h"
#include "Optimizer.h"
#include "VM.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./vm <input.mylang> [--verbose]\n";
        std::cerr << "  --verbose — show each instruction execution\n";
        return 1;
    }

    std::string inputFile = argv[1];
    bool verbose = false;
    for (int i = 2; i < argc; i++)
        if (std::string(argv[i]) == "--verbose") verbose = true;

    std::ifstream file(inputFile);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open '" << inputFile << "'\n";
        return 1;
    }
    std::string source((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    try {
        std::istringstream ss(source);
        Lexer         lexer(ss);
        SymbolTable   symTable;
        ProgramParser parser(lexer, symTable);
        auto ast = parser.parseProgram();
        std::cout << " Parsing done\n";

        IRGenerator irGen;
        TACProgram  tac = irGen.generate(ast.get());
        std::cout << " IR Generation done\n";

        Optimizer  optimizer;
        TACProgram optimized = optimizer.optimize(tac);
        std::cout << " Optimization done\n";

        std::cout << "\n═══ VM / INTERPRETER ═══\n";
        VM vm;
        vm.load(optimized);
        vm.run(verbose);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}