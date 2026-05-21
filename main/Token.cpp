#include "Token.h"


Token::Token()
    : type(TokenType::EndOfExpr), value(""), line(0), column(0) {}

Token::Token(TokenType t, const std::string& v, int ln, int col)
    : type(t), value(v), line(ln), column(col) {}

std::string Token::toString() const {
    return "[" + value + " @ " + std::to_string(line) + ":" + std::to_string(column) + "]";
}
