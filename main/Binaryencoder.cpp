#include "BinaryEncoder.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <iomanip>


static const int OP_LOAD   = 0b0000011;  
static const int OP_STORE  = 0b0100011;  
static const int OP_IMM    = 0b0010011;  
static const int OP_REG    = 0b0110011;  
static const int OP_BRANCH = 0b1100011; 
static const int OP_JAL    = 0b1101111;  
static const int OP_JALR   = 0b1100111;  
static const int OP_LUI    = 0b0110111;  
static const int OP_AUIPC  = 0b0010111;  


BinaryEncoder::BinaryEncoder() {}

std::string BinaryEncoder::trim(const std::string& s) const {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::vector<std::string> BinaryEncoder::splitLines(const std::string& text) const {
    std::vector<std::string> lines;
    std::istringstream ss(text);
    std::string line;
    while (std::getline(ss, line))
        lines.push_back(trim(line));
    return lines;
}

std::vector<std::string> BinaryEncoder::tokenize(const std::string& line) const {
    std::vector<std::string> tokens;
    std::string cur;

    for (char c : line) {
        if (c == '#') break;          
        if (c == ',' || c == ' ' || c == '\t') {
            if (!cur.empty()) { tokens.push_back(cur); cur.clear(); }
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) tokens.push_back(cur);
    return tokens;
}

bool BinaryEncoder::isLabel(const std::string& s) const {
    return !s.empty() && s.back() == ':';
}

bool BinaryEncoder::isDirective(const std::string& s) const {
    return !s.empty() && s[0] == '.';
}


int BinaryEncoder::parseReg(const std::string& s) const {
    // ABI names
    if (s == "zero") return 0;
    if (s == "ra")   return 1;
    if (s == "sp")   return 2;
    if (s == "gp")   return 3;
    if (s == "tp")   return 4;
    if (s == "fp" || s == "s0") return 8;
    if (s == "s1")   return 9;

    if (s == "t0")   return 5;
    if (s == "t1")   return 6;
    if (s == "t2")   return 7;

    if (s == "t3")   return 28;
    if (s == "t4")   return 29;
    if (s == "t5")   return 30;
    if (s == "t6")   return 31;

    if (s == "a0")   return 10;
    if (s == "a1")   return 11;
    if (s == "a2")   return 12;
    if (s == "a3")   return 13;
    if (s == "a4")   return 14;
    if (s == "a5")   return 15;
    if (s == "a6")   return 16;
    if (s == "a7")   return 17;

    if (s == "s2")   return 18;
    if (s == "s3")   return 19;

    if (s.size() > 1 && s[0] == 'x')
        return std::stoi(s.substr(1));

    throw std::runtime_error("BinaryEncoder: unknown register '" + s + "'");
}

int BinaryEncoder::parseImm(const std::string& s) const {
    if (s.empty()) return 0;
    if (s.size() > 2 && s[0]=='0' && s[1]=='x')
        return std::stoi(s, nullptr, 16);
    return std::stoi(s);
}


void BinaryEncoder::parseOffset(const std::string& s, int& imm, int& reg) const {
    size_t lp = s.find('(');
    size_t rp = s.find(')');
    if (lp == std::string::npos || rp == std::string::npos)
        throw std::runtime_error("BinaryEncoder: invalid offset format '" + s + "'");
    imm = parseImm(s.substr(0, lp));
    reg = parseReg(s.substr(lp + 1, rp - lp - 1));
}

uint32_t BinaryEncoder::encodeR(int opcode, int rd, int funct3,
                                 int rs1, int rs2, int funct7) {
    return ((funct7 & 0x7F) << 25) |
           ((rs2    & 0x1F) << 20) |
           ((rs1    & 0x1F) << 15) |
           ((funct3 & 0x07) << 12) |
           ((rd     & 0x1F) <<  7) |
           ( opcode & 0x7F);
}

uint32_t BinaryEncoder::encodeI(int opcode, int rd, int funct3,
                                 int rs1, int imm) {
    return ((imm    & 0xFFF) << 20) |
           ((rs1    & 0x1F)  << 15) |
           ((funct3 & 0x07)  << 12) |
           ((rd     & 0x1F)  <<  7) |
           ( opcode & 0x7F);
}

uint32_t BinaryEncoder::encodeS(int rs1, int rs2, int funct3, int imm) {
    return (((imm >> 5) & 0x7F) << 25) |
           ((rs2        & 0x1F) << 20) |
           ((rs1        & 0x1F) << 15) |
           ((funct3     & 0x07) << 12) |
           (( imm       & 0x1F) <<  7) |
           OP_STORE;
}

uint32_t BinaryEncoder::encodeB(int rs1, int rs2, int funct3, int imm) {
    return (((imm >> 12) & 0x1)  << 31) |
           (((imm >>  5) & 0x3F) << 25) |
           ((rs2         & 0x1F) << 20) |
           ((rs1         & 0x1F) << 15) |
           ((funct3      & 0x07) << 12) |
           (((imm >>  1) & 0xF)  <<  8) |
           (((imm >> 11) & 0x1)  <<  7) |
           OP_BRANCH;
}

uint32_t BinaryEncoder::encodeJ(int rd, int imm) {
    return (((imm >> 20) & 0x1)   << 31) |
           (((imm >>  1) & 0x3FF) << 21) |
           (((imm >> 11) & 0x1)   << 20) |
           (((imm >> 12) & 0xFF)  << 12) |
           ((rd          & 0x1F)  <<  7) |
           OP_JAL;
}

uint32_t BinaryEncoder::encodeADDI(int rd, int rs1, int imm) {
    return encodeI(OP_IMM, rd, 0b000, rs1, imm);
}
uint32_t BinaryEncoder::encodeLW(int rd, int rs1, int imm) {
    return encodeI(OP_LOAD, rd, 0b010, rs1, imm);
}
uint32_t BinaryEncoder::encodeSW(int rs1, int rs2, int imm) {
    return encodeS(rs1, rs2, 0b010, imm);
}
uint32_t BinaryEncoder::encodeADD(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b000, rs1, rs2, 0b0000000);
}
uint32_t BinaryEncoder::encodeSUB(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b000, rs1, rs2, 0b0100000);
}
uint32_t BinaryEncoder::encodeMUL(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b000, rs1, rs2, 0b0000001);
}
uint32_t BinaryEncoder::encodeDIV(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b100, rs1, rs2, 0b0000001);
}
uint32_t BinaryEncoder::encodeSLT(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b010, rs1, rs2, 0b0000000);
}
uint32_t BinaryEncoder::encodeSLTU(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b011, rs1, rs2, 0b0000000);
}
uint32_t BinaryEncoder::encodeXOR(int rd, int rs1, int rs2) {
    return encodeR(OP_REG, rd, 0b100, rs1, rs2, 0b0000000);
}
uint32_t BinaryEncoder::encodeXORI(int rd, int rs1, int imm) {
    return encodeI(OP_IMM, rd, 0b100, rs1, imm);
}
uint32_t BinaryEncoder::encodeSLTIU(int rd, int rs1, int imm) {
    return encodeI(OP_IMM, rd, 0b011, rs1, imm);
}
uint32_t BinaryEncoder::encodeBEQ(int rs1, int rs2, int imm) {
    return encodeB(rs1, rs2, 0b000, imm);
}
uint32_t BinaryEncoder::encodeBNE(int rs1, int rs2, int imm) {
    return encodeB(rs1, rs2, 0b001, imm);
}
uint32_t BinaryEncoder::encodeJAL(int rd, int imm) {
    return encodeJ(rd, imm);
}
uint32_t BinaryEncoder::encodeJALR(int rd, int rs1, int imm) {
    return encodeI(OP_JALR, rd, 0b000, rs1, imm);
}
uint32_t BinaryEncoder::encodeLUI(int rd, int imm) {
    return ((imm & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | OP_LUI;
}
uint32_t BinaryEncoder::encodeAUIPC(int rd, int imm) {
    return ((imm & 0xFFFFF) << 12) | ((rd & 0x1F) << 7) | OP_AUIPC;
}

std::vector<uint32_t> BinaryEncoder::encodePseudo(
    const std::string& mnemonic,
    const std::vector<std::string>& ops,
    int currentAddr)
{
    std::vector<uint32_t> result;

    if (mnemonic == "mv") {
        result.push_back(encodeADDI(parseReg(ops[0]), parseReg(ops[1]), 0));
    }
    else if (mnemonic == "li") {
        int rd  = parseReg(ops[0]);
        int imm = parseImm(ops[1]);
        if (imm >= -2048 && imm <= 2047) {
            result.push_back(encodeADDI(rd, 0, imm));
        } else {
            int hi = (imm >> 12) & 0xFFFFF;
            int lo = imm & 0xFFF;
            if (lo & 0x800) { hi++; lo |= 0xFFFFF000; }
            result.push_back(encodeLUI(rd, hi));
            result.push_back(encodeADDI(rd, rd, lo));
        }
    }
    else if (mnemonic == "neg") {
        result.push_back(encodeSUB(parseReg(ops[0]), 0, parseReg(ops[1])));
    }
    else if (mnemonic == "ret") {
        result.push_back(encodeJALR(0, 1, 0));
    }
    else if (mnemonic == "j") {
        auto it = labelTable.find(ops[0]);
        if (it == labelTable.end())
            throw std::runtime_error("BinaryEncoder: unknown label '" + ops[0] + "'");
        int offset = it->second - currentAddr;
        result.push_back(encodeJAL(0, offset));
    }
    else if (mnemonic == "call") {
        auto it = labelTable.find(ops[0]);
        if (it == labelTable.end())
            throw std::runtime_error("BinaryEncoder: unknown label '" + ops[0] + "'");
        int offset = it->second - currentAddr;
        int hi = (offset >> 12) & 0xFFFFF;
        int lo = offset & 0xFFF;
        if (lo & 0x800) hi++;
        result.push_back(encodeAUIPC(1, hi));
        result.push_back(encodeJALR(1, 1, lo));
    }
    else if (mnemonic == "seqz") {
        result.push_back(encodeSLTIU(parseReg(ops[0]), parseReg(ops[1]), 1));
    }
    else if (mnemonic == "snez") {
        result.push_back(encodeSLTU(parseReg(ops[0]), 0, parseReg(ops[1])));
    }

    return result;
}

void BinaryEncoder::buildLabelTable(const std::vector<std::string>& lines) {
    labelTable.clear();
    int instrAddr = 0;  

    for (const std::string& line : lines) {
        if (line.empty() || isDirective(line)) continue;

        if (isLabel(line)) {
            std::string lbl = line.substr(0, line.size() - 1);
            labelTable[lbl] = instrAddr;
        } else {
            auto tokens = tokenize(line);
            if (tokens.empty()) continue;
            std::string mn = tokens[0];
            if (mn == "li") {
                int imm = tokens.size() > 2 ? parseImm(tokens[2]) : 0;
                instrAddr += (imm >= -2048 && imm <= 2047) ? 4 : 8;
            } else if (mn == "call") {
                instrAddr += 8;  
            } else {
                instrAddr += 4;
            }
        }
    }
}

std::vector<uint32_t> BinaryEncoder::encodeLines(
    const std::vector<std::string>& lines)
{
    std::vector<uint32_t> result;
    int currentAddr = 0;

    for (const std::string& line : lines) {
        if (line.empty() || isLabel(line) || isDirective(line)) continue;

        auto tokens = tokenize(line);
        if (tokens.empty()) continue;

        std::string mn = tokens[0];
        std::vector<std::string> ops(tokens.begin() + 1, tokens.end());

        try {
            if (mn=="mv"||mn=="li"||mn=="neg"||mn=="ret"||
                mn=="j" ||mn=="call"||mn=="seqz"||mn=="snez") {
                auto encoded = encodePseudo(mn, ops, currentAddr);
                for (uint32_t w : encoded) { result.push_back(w); currentAddr+=4; }
                continue;
            }

            uint32_t word = 0;

            if (mn == "addi") {
                word = encodeADDI(parseReg(ops[0]), parseReg(ops[1]), parseImm(ops[2]));
            }
            else if (mn == "lw") {
                int imm, rs1;
                parseOffset(ops[1], imm, rs1);
                word = encodeLW(parseReg(ops[0]), rs1, imm);
            }
            else if (mn == "sw") {
                int imm, rs1;
                parseOffset(ops[1], imm, rs1);
                word = encodeSW(rs1, parseReg(ops[0]), imm);
            }
            else if (mn == "add")  word = encodeADD(parseReg(ops[0]),parseReg(ops[1]),parseReg(ops[2]));
            else if (mn == "sub")  word = encodeSUB(parseReg(ops[0]),parseReg(ops[1]),parseReg(ops[2]));
            else if (mn == "mul")  word = encodeMUL(parseReg(ops[0]),parseReg(ops[1]),parseReg(ops[2]));
            else if (mn == "div")  word = encodeDIV(parseReg(ops[0]),parseReg(ops[1]),parseReg(ops[2]));
            else if (mn == "slt")  word = encodeSLT(parseReg(ops[0]),parseReg(ops[1]),parseReg(ops[2]));
            else if (mn == "xor")  word = encodeXOR(parseReg(ops[0]),parseReg(ops[1]),parseReg(ops[2]));
            else if (mn == "xori") word = encodeXORI(parseReg(ops[0]),parseReg(ops[1]),parseImm(ops[2]));
            else if (mn == "beq") {
                auto it = labelTable.find(ops[2]);
                if (it==labelTable.end()) throw std::runtime_error("unknown label '"+ops[2]+"'");
                word = encodeBEQ(parseReg(ops[0]),parseReg(ops[1]),it->second-currentAddr);
            }
            else if (mn == "bne") {
                auto it = labelTable.find(ops[2]);
                if (it==labelTable.end()) throw std::runtime_error("unknown label '"+ops[2]+"'");
                word = encodeBNE(parseReg(ops[0]),parseReg(ops[1]),it->second-currentAddr);
            }
            else if (mn == "jal") {
                auto it = labelTable.find(ops[1]);
                if (it==labelTable.end()) throw std::runtime_error("unknown label '"+ops[1]+"'");
                word = encodeJAL(parseReg(ops[0]),it->second-currentAddr);
            }
            else if (mn == "jalr") {
                int imm, rs1;
                parseOffset(ops[1], imm, rs1);
                word = encodeJALR(parseReg(ops[0]), rs1, imm);
            }
            else {
                continue;
            }

            result.push_back(word);
            currentAddr += 4;

        } catch (const std::exception& e) {
            std::cerr << "  Warning: " << e.what() << " → line: '" << line << "'\n";
            currentAddr += 4;
        }
    }

    return result;
}

bool BinaryEncoder::encode(const std::string& assembly,
                           const std::string& outFile) {
    std::cout << "\n═══ BINARY ENCODER ═══\n";

    auto lines = splitLines(assembly);

    buildLabelTable(lines);
    std::cout << "  Labels found: " << labelTable.size() << "\n";

    auto binary = encodeLines(lines);
    std::cout << "  Instructions encoded: " << binary.size() << "\n";

    std::ofstream file(outFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "  Error: cannot create '" << outFile << "'\n";
        return false;
    }

    for (uint32_t word : binary) {
        uint8_t bytes[4] = {
            uint8_t(word & 0xFF),
            uint8_t((word >> 8)  & 0xFF),
            uint8_t((word >> 16) & 0xFF),
            uint8_t((word >> 24) & 0xFF)
        };
        file.write(reinterpret_cast<char*>(bytes), 4);
    }
    file.close();

    std::cout << "  ✓ Binary written to '" << outFile << "'\n";
    std::cout << "  Total size: " << binary.size() * 4 << " bytes\n";

    printBinary(binary);
    return true;
}

void BinaryEncoder::printBinary(const std::vector<uint32_t>& code) const {
    std::cout << "\n  Address  │ Binary (32-bit)          │ Hex\n";
    std::cout << "  ─────────┼──────────────────────────┼────────────\n";

    for (size_t i = 0; i < code.size(); i++) {
        uint32_t w = code[i];
        std::cout << "  0x" << std::hex << std::setw(4) << std::setfill('0')
                  << (i * 4) << "   │ ";

        for (int b = 31; b >= 0; b--) {
            std::cout << ((w >> b) & 1);
            if (b % 4 == 0 && b > 0) std::cout << " ";
        }
        std::cout << " │ 0x" << std::setw(8) << w << "\n";
    }
    std::cout << std::dec;
}