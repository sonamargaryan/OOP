#pragma once
#include <string>
#include <ostream>

//    fc.writeAssembly(assemblyCode);   // output.s գրել
//    fc.assemble();                    // output.o ստեղծել
//    fc.link();                        // output ստեղծել
//    fc.run();                         // (optional) գործարկել

class FileCreator {
public:
    explicit FileCreator(const std::string& baseName);

    bool writeAssembly(const std::string& assemblyCode);

    bool assemble();

    bool link();

    bool createExecutable(const std::string& assemblyCode);

    std::string getAsmFile()  const { return baseName + ".s"; }
    std::string getObjFile()  const { return baseName + ".o"; }
    std::string getExecFile() const { return baseName; }

private:
    std::string baseName;

    bool runCommand(const std::string& cmd, const std::string& stepName);
};