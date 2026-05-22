#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "ProgramParser.h"
#include "IRGenerator.h"
#include "Optimizer.h"
#include "CodeGenerator.h"
#include "FileCreator.h"
#include "VM.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./compiler <input.mylang> [output] [--run]\n";
        return 1;
    }

    std::string outputName = (argc >= 3 && std::string(argv[2]) != "--run")
                              ? argv[2] : "output";
    bool runVM = false;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--run") runVM = true;

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: cannot open '" << argv[1] << "'\n";
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
        std::cout << "✓ Parsing done\n";

        IRGenerator irGen;
        TACProgram  tac = irGen.generate(ast.get());
        irGen.printTAC(tac);
        std::cout << "\n✓ IR Generation done\n";

        Optimizer  optimizer;
        TACProgram optimized = optimizer.optimize(tac);
        std::cout << "\n═══ OPTIMIZED TAC ═══\n";
        irGen.printTAC(optimized);
        std::cout << "\n✓ Optimization done\n";

        std::cout << "\n═══ RISC-V ASSEMBLY ═══\n";
        std::ostringstream asmStream;
        CodeGenerator codegen(asmStream);
        codegen.generate(optimized);
        std::cout << asmStream.str();
        std::cout << "\n✓ Code Generation done\n";

        FileCreator fc(outputName);
        fc.createExecutable(asmStream.str());

        if (runVM) {
            std::cout << "\n═══ VM (Interpreter) ═══\n";
            VM vm;
            
            vm.load(optimized);
            vm.run(true);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}