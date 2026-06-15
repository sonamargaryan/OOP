#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

// ════════════════════════════════════════════════════════════
//  BINARY ENCODER — RISC-V Assembly → 32-bit Binary
//
//  RISC-V RV32I instruction formats.
//
//  R-type:  funct7 | rs2 | rs1 | funct3 | rd | opcode
//  I-type:  imm[11:0] | rs1 | funct3 | rd | opcode
//  S-type:  imm[11:5] | rs2 | rs1 | funct3 | imm[4:0] | opcode
//  B-type:  imm[12|10:5] | rs2 | rs1 | funct3 | imm[4:1|11] | opcode
//  J-type:  imm[20|10:1|11|19:12] | rd | opcode
// ════════════════════════════════════════════════════════════
class BinaryEncoder {
public:
    BinaryEncoder();

    bool encode(const std::string& assembly, const std::string& outFile);

    void printBinary(const std::vector<uint32_t>& code) const;

private:
    void buildLabelTable(const std::vector<std::string>& lines);

    std::vector<uint32_t> encodeLines(const std::vector<std::string>& lines);

    // ── Instruction encoders ───────────────────────────────
    uint32_t encodeR(int opcode, int rd, int funct3,
                     int rs1, int rs2, int funct7);
    uint32_t encodeI(int opcode, int rd, int funct3,
                     int rs1, int imm);
    uint32_t encodeS(int rs1, int rs2, int funct3, int imm);
    uint32_t encodeB(int rs1, int rs2, int funct3, int imm);
    uint32_t encodeJ(int rd, int imm);

    // ── Instruction-specific encoders ──────────────────────
    uint32_t encodeADDI(int rd, int rs1, int imm);
    uint32_t encodeLW(int rd, int rs1, int imm);
    uint32_t encodeSW(int rs1, int rs2, int imm);
    uint32_t encodeADD(int rd, int rs1, int rs2);
    uint32_t encodeSUB(int rd, int rs1, int rs2);
    uint32_t encodeMUL(int rd, int rs1, int rs2);
    uint32_t encodeDIV(int rd, int rs1, int rs2);
    uint32_t encodeSLT(int rd, int rs1, int rs2);
    uint32_t encodeSLTIU(int rd, int rs1, int imm);
    uint32_t encodeSLTU(int rd, int rs1, int rs2);
    uint32_t encodeXOR(int rd, int rs1, int rs2);
    uint32_t encodeXORI(int rd, int rs1, int imm);
    uint32_t encodeBEQ(int rs1, int rs2, int imm);
    uint32_t encodeBNE(int rs1, int rs2, int imm);
    uint32_t encodeJAL(int rd, int imm);
    uint32_t encodeJALR(int rd, int rs1, int imm);
    uint32_t encodeLUI(int rd, int imm);
    uint32_t encodeAUIPC(int rd, int imm);


    std::vector<uint32_t> encodePseudo(
        const std::string& mnemonic,
        const std::vector<std::string>& ops,
        int currentAddr);

    int parseReg(const std::string& s) const;
    int parseImm(const std::string& s) const;
    void parseOffset(const std::string& s, int& imm, int& reg) const;

    std::vector<std::string> tokenize(const std::string& line) const;
    std::vector<std::string> splitLines(const std::string& text) const;
    std::string              trim(const std::string& s) const;
    bool                     isLabel(const std::string& s) const;
    bool                     isDirective(const std::string& s) const;

    std::unordered_map<std::string, int> labelTable;
};