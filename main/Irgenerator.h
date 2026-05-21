#pragma once
#include "ASTNode.h"
#include "TACInstruction.h"
#include <string>

class IRGenerator {
public:
    IRGenerator();

    TACProgram generate(ASTNode* node);

    void printTAC(const TACProgram& program);

private:
    TACProgram  code;        
    int         tempCount;   
    int         labelCount;  

    std::string newTemp();               
    std::string newLabel(const std::string& prefix = "L");

    void emit(const TACInstruction& instr);

    void        genProgram(ProgramNode*     node);
    void        genFunction(FunctionDefNode* node);
    void        genBlock(BlockNode*         node);
    void        genStatement(ASTNode*       node);
    void        genVarDecl(VarDeclNode*     node);
    void        genReturn(ReturnNode*       node);
    void        genIf(IfNode*              node);
    void        genWhile(WhileNode*        node);
    void        genDoWhile(DoWhileNode*    node);

    std::string genExpr(ASTNode*           node);
    std::string genBinaryOp(BinaryOpNode*  node);

    TACOp       charToTACOp(char op);
};