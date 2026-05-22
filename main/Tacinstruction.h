#pragma once
#include <string>
#include <vector>

enum class TACOp {
    ADD,        
    SUB,        
    MUL,        
    DIV,        
    NEG,        

    LT,         
    GT,         
    LE,         
    GE,         
    EQ,         
    NE,         

    COPY,    

    
    GOTO,       
    IFFALSE,    
    IFTRUE,     
    LABEL,      

    FUNC_BEGIN, 
    FUNC_END,   
    PARAM,      
    CALL,       
    RETURN,     
};

class TACInstruction {
public:
    TACOp       op;
    std::string dest;   
    std::string src1;   
    std::string src2;  

    TACInstruction(TACOp op,
                   const std::string& dest = "",
                   const std::string& src1 = "",
                   const std::string& src2 = "");

    std::string toString() const;

private:
    static std::string opName(TACOp op);
};

using TACProgram = std::vector<TACInstruction>;