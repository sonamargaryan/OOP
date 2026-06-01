#include "Filecreator.h"
#include "Binaryencoder.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

FileCreator::FileCreator(const std::string& baseName)
    : baseName(baseName) {}
bool FileCreator::runCommand(const std::string& cmd,
                             const std::string& stepName) {
    std::cout << "  [" << stepName << "] " << cmd << "\n";
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "  Error: " << stepName << " failed (code " << ret << ")\n";
        return false;
    }
    std::cout << "  ✓ " << stepName << " done\n";
    return true;
}

bool FileCreator::writeAssembly(const std::string& assemblyCode) {
    std::string filename = getAsmFile();
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "  Error: cannot create '" << filename << "'\n";
        return false;
    }

    file << assemblyCode;
    file.close();

    std::cout << "  ✓ Assembly written to '" << filename << "'\n";
    return true;
}

bool FileCreator::assemble() {
    std::string cmd = "riscv64-unknown-elf-as"
                      " -march=rv32im"
                      " -mabi=ilp32"
                      " " + getAsmFile() +
                      " -o " + getObjFile();

    return runCommand(cmd, "Assembler");
}
bool FileCreator::encodeBinary(const std::string& assemblyCode) {
    BinaryEncoder encoder;
    return encoder.encode(assemblyCode, getBinFile());
}
bool FileCreator::createExecutable(const std::string& assemblyCode) {
    std::cout << "\n═══ FILE CREATOR ═══\n";

    if (!writeAssembly(assemblyCode)) return false;
    encodeBinary(assemblyCode);
    assemble();
    std::cout << "✓ Files created: "
              << getAsmFile() << ", "
              << getBinFile() << "\n";
    return true;
}