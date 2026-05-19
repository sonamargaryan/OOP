#pragma once
#include <string>

enum class TokenType {
    EndOfExpr   = 0,   
    Number      = 1,   
    Name        = 2,   
    Operator    = 3,   
    OpenParen   = 4,   
    CloseParen  = 5,   
    Assignment  = 6,   
    Comparison  = 7,   
    Semicolon   = 8,   
    Comma       = 9,   

    Keyword,           
    OpenBrace,         
    CloseBrace,        

    Unknown
};

class Token {
public:
    TokenType   type;
    std::string value;
    int         line;
    int         column;

    Token();
    Token(TokenType t, const std::string& v, int ln, int col);

    std::string toString() const;
};
