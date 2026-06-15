#pragma once
#include "Memory.h"
#include "Processor.h"
#include "TACInstruction.h"
#include <string>
#include <vector>

// Interactive command-line debugger for the TAC VM.
//
// Commands:
//   run / r                 - run until breakpoint or program end
//   step / s [n]            - execute n instructions (default 1)
//   continue / c            - alias for run
//   break / b <label|func>  - set breakpoint at label or function name
//   break / b <addr>        - set breakpoint at numeric IP
//   delete / d <n>          - remove breakpoint n (see 'breakpoints')
//   breakpoints / bl        - list breakpoints
//   print / p <name>        - print value of a variable
//   regs                    - print non-zero registers
//   list / l [n]            - list n instructions around current IP
//   backtrace / bt          - show call stack
//   info                    - show current IP / instruction / frame
//   help / h                - show command list
//   quit / q                - exit debugger
class Debugger {
public:
    Debugger(const TACProgram& program, Memory& memory, Processor& processor);

    void run();  // start interactive REPL

private:
    const TACProgram& program;
    Memory&           memory;
    Processor&        processor;

    std::vector<uint32_t> breakpoints;

    void printBanner();
    void printHelp() const;
    void printInstruction(uint32_t ip) const;
    void listInstructions(uint32_t center, int count) const;
    void printRegistersInfo() const;
    void printVariable(const std::string& name) const;
    void printBacktrace() const;
    void printBreakpoints() const;
    void printInfo() const;

    bool resolveAddress(const std::string& token, uint32_t& addr) const;
    bool atBreakpoint(uint32_t ip) const;

    // returns false if program finished
    bool doStep(int count);
    bool doNext(int count);
    bool doContinue();
};
