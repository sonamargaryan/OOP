#include "Debugger.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

Debugger::Debugger(const TACProgram& program, Memory& memory, Processor& processor)
    : program(program), memory(memory), processor(processor) {}

void Debugger::printBanner() {
    std::cout << "\n═══ DEBUGGER ═══\n";
    std::cout << "  " << program.size() << " instructions loaded. Type 'help' for commands.\n";
}

void Debugger::printHelp() const {
    std::cout <<
        "  run, r              - run until breakpoint or program end\n"
        "  step, s [n]         - execute n instructions (Step Into, default 1)\n"
        "  next, n [n]         - step over CALL instructions (Step Over)\n"
        "  continue, c         - continue execution until breakpoint/end\n"
        "  break, b <label|fn> - set breakpoint at label, function, or address\n"
        "  delete, d <n>       - remove breakpoint number n\n"
        "  breakpoints, bl     - list breakpoints\n"
        "  print, p <var>      - print value of variable\n"
        "  regs                - print non-zero registers\n"
        "  list, l [n]         - list n instructions around current IP\n"
        "  backtrace, bt       - show call stack\n"
        "  info                - show current IP / instruction / frame depth\n"
        "  help, h             - show this help\n"
        "  quit, q             - exit debugger\n";
}

void Debugger::printInstruction(uint32_t ip) const {
    if (ip >= memory.codeSize()) {
        std::cout << "  (end of program)\n";
        return;
    }
    std::cout << "  [IP=" << ip << "] " << memory.fetchInstruction(ip).toString() << "\n";
}

void Debugger::listInstructions(uint32_t center, int count) const {
    int32_t start = (int32_t)center - count / 2;
    if (start < 0) start = 0;
    uint32_t end = std::min(memory.codeSize(), (uint32_t)(start + count));

    for (uint32_t i = (uint32_t)start; i < end; i++) {
        std::string marker = (i == center) ? "-> " : "   ";
        std::string bp     = atBreakpoint(i) ? "* " : "  ";
        std::cout << marker << bp << "[" << i << "] "
                  << memory.fetchInstruction(i).toString() << "\n";
    }
}

void Debugger::printRegistersInfo() const {
    processor.printRegisters();
}

void Debugger::printVariable(const std::string& name) const {
    if (!memory.stackEmpty()) {
        const auto& locals = memory.topFrame().locals;
        auto it = locals.find(name);
        if (it != locals.end()) {
            std::cout << "  " << name << " = " << it->second << "  (local)\n";
            return;
        }
    }
    if (memory.dataExists(name)) {
        std::cout << "  " << name << " = " << memory.loadData(name) << "  (global)\n";
        return;
    }
    std::cout << "  variable '" << name << "' not found in current frame or globals\n";
}

void Debugger::printBacktrace() const {
    auto frames = memory.getCallStack();
    if (frames.empty()) {
        std::cout << "  (no active frames)\n";
        return;
    }
    std::cout << "  Call stack (top first):\n";
    for (size_t i = 0; i < frames.size(); i++) {
        std::cout << "    #" << i << " " << frames[i].funcName
                  << "  (return IP=" << frames[i].returnIP << ")\n";
        for (const auto& kv : frames[i].locals)
            std::cout << "         " << kv.first << " = " << kv.second << "\n";
    }
}

void Debugger::printBreakpoints() const {
    if (breakpoints.empty()) {
        std::cout << "  (no breakpoints set)\n";
        return;
    }
    for (size_t i = 0; i < breakpoints.size(); i++) {
        std::cout << "  #" << i << "  IP=" << breakpoints[i] << "  ";
        printInstruction(breakpoints[i]);
    }
}

void Debugger::printInfo() const {
    uint32_t ip = processor.getCurrentIP();
    std::cout << "  IP        = " << ip << "\n";
    std::cout << "  Stack depth = " << memory.stackDepth() << "\n";
    std::cout << "  Next instruction:\n  ";
    printInstruction(ip);
}

bool Debugger::resolveAddress(const std::string& token, uint32_t& addr) const {
    // numeric address
    if (!token.empty() && std::all_of(token.begin(), token.end(), ::isdigit)) {
        addr = (uint32_t)std::stoul(token);
        return true;
    }
    // try function name
    if (memory.hasFunction(token)) {
        addr = memory.getFuncAddress(token);
        return true;
    }
    // try label
    try {
        addr = memory.getLabelAddress(token);
        return true;
    } catch (...) {
        return false;
    }
}

bool Debugger::atBreakpoint(uint32_t ip) const {
    return std::find(breakpoints.begin(), breakpoints.end(), ip) != breakpoints.end();
}

bool Debugger::doStep(int count) {
    for (int i = 0; i < count; i++) {
        if (!processor.isRunning() || processor.getCurrentIP() >= memory.codeSize()) {
            std::cout << "  Program finished.\n";
            return false;
        }
        printInstruction(processor.getCurrentIP());
        processor.step();
    }
    if (!processor.isRunning()) {
        std::cout << "  Program finished.\n";
        return false;
    }
    return true;
}

bool Debugger::doNext(int count) {
    for (int i = 0; i < count; i++) {
        if (!processor.isRunning() || processor.getCurrentIP() >= memory.codeSize()) {
            std::cout << "  Program finished.\n";
            return false;
        }

        uint32_t ip = processor.getCurrentIP();
        const TACInstruction& instr = memory.fetchInstruction(ip);
        printInstruction(ip);

        if (instr.op == TACOp::CALL) {
            int depthBefore = memory.stackDepth();
            processor.step();  // executes CALL, pushes new frame
            // run until the callee's frame is popped (RETURN brings us back)
            while (processor.isRunning() && memory.stackDepth() > depthBefore)
                processor.step();
        } else {
            processor.step();
        }
    }
    if (!processor.isRunning()) {
        std::cout << "  Program finished.\n";
        return false;
    }
    return true;
}

bool Debugger::doContinue() {
    if (!processor.isRunning() || processor.getCurrentIP() >= memory.codeSize()) {
        std::cout << "  Program already finished.\n";
        return false;
    }

    // Execute the instruction we're currently sitting on first, so a
    // 'continue' issued right after hitting a breakpoint doesn't
    // immediately re-trigger the same breakpoint.
    processor.step();

    while (processor.isRunning() && processor.getCurrentIP() < memory.codeSize()) {
        uint32_t ip = processor.getCurrentIP();
        if (atBreakpoint(ip)) {
            std::cout << "  Hit breakpoint at IP=" << ip << "\n";
            printInstruction(ip);
            return true;
        }
        processor.step();
    }
    std::cout << "  Program finished.\n";
    return false;
}

void Debugger::run() {
    printBanner();
    processor.init();

    std::string line;
    std::string lastCmd;

    while (true) {
        std::cout << "(dbg) ";
        if (!std::getline(std::cin, line)) break;

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd.empty())
            cmd = lastCmd;  // repeat last command on empty input
        if (cmd.empty())
            continue;

        lastCmd = cmd;

        if (cmd == "help" || cmd == "h") {
            printHelp();
        } else if (cmd == "quit" || cmd == "q" || cmd == "exit") {
            break;
        } else if (cmd == "run" || cmd == "r" || cmd == "continue" || cmd == "c") {
            doContinue();
        } else if (cmd == "step" || cmd == "s") {
            int n = 1;
            if (iss >> n) {} else n = 1;
            doStep(n);
        } else if (cmd == "next" || cmd == "n") {
            int n = 1;
            if (iss >> n) {} else n = 1;
            doNext(n);
        } else if (cmd == "break" || cmd == "b") {
            std::string target;
            if (!(iss >> target)) {
                std::cout << "  usage: break <label|function|address>\n";
                continue;
            }
            uint32_t addr;
            if (resolveAddress(target, addr)) {
                if (atBreakpoint(addr)) {
                    std::cout << "  breakpoint already set at IP=" << addr << "\n";
                } else {
                    breakpoints.push_back(addr);
                    std::cout << "  breakpoint #" << breakpoints.size() - 1
                              << " set at IP=" << addr << "\n";
                }
            } else {
                std::cout << "  could not resolve '" << target << "'\n";
            }
        } else if (cmd == "delete" || cmd == "d") {
            size_t n;
            if (!(iss >> n) || n >= breakpoints.size()) {
                std::cout << "  usage: delete <breakpoint number>\n";
                continue;
            }
            breakpoints.erase(breakpoints.begin() + n);
            std::cout << "  breakpoint #" << n << " removed\n";
        } else if (cmd == "breakpoints" || cmd == "bl") {
            printBreakpoints();
        } else if (cmd == "print" || cmd == "p") {
            std::string var;
            if (!(iss >> var)) {
                std::cout << "  usage: print <variable>\n";
                continue;
            }
            printVariable(var);
        } else if (cmd == "regs") {
            printRegistersInfo();
        } else if (cmd == "list" || cmd == "l") {
            int n = 10;
            if (iss >> n) {} else n = 10;
            listInstructions(processor.getCurrentIP(), n);
        } else if (cmd == "backtrace" || cmd == "bt") {
            printBacktrace();
        } else if (cmd == "info") {
            printInfo();
        } else {
            std::cout << "  unknown command '" << cmd << "' (type 'help')\n";
        }
    }

    std::cout << "  Exiting debugger.\n";
}
