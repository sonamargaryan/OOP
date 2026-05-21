#pragma once
#include "Token.h"
#include <istream>
#include <string>
#include <vector>
#include <unordered_map>

enum class LexerState {
    Start      = 0,   // սպասում ենք նոր token-ի
    InNumber   = 1,   // կարդում ենք թիվ        
    InName     = 2,   // կարդում ենք բառ         
    InOperator = 3    // կարդում ենք operator 
};

class Lexer {
public:
    explicit Lexer(std::istream& input);

    Token getNextToken();              
    void  pushBack(const Token& tok); 
    void  reset();

    int currentLine()   const { return line; }
    int currentColumn() const { return column; }

private:
    std::istream& src;         
    LexerState    state;       
    std::string   buffer;      
    int           line;        
    int           column;      

    std::vector<Token>                         pending;  
    std::unordered_map<std::string, TokenType> kwMap;    

    static const int N_STATES = 4;
    static const int N_CHARS  = 7;

    enum class CharKind {
        Digit  = 0,   
        Letter = 1,   
        Space  = 2,   
        OpChar = 3,   
        Fence  = 4,   
        Sep    = 5,
        Other  = 6
    };

    LexerState transitions[N_STATES][N_CHARS];

    void      buildTransitions();
    void      loadKeywords();
    CharKind  classify(char c) const;
    Token     buildToken();           
    Token     singleChar(char c);  
    char      comparisonCode(const std::string& op) const;
};
