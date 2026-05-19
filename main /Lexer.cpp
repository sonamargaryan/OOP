#include "Lexer.h"
#include <cctype>
#include <stdexcept>

Lexer::Lexer(std::istream& input)
    : src(input), state(LexerState::Start), line(1), column(0)
{
    buildTransitions();
    loadKeywords();
}

void Lexer::buildTransitions() {
    for (int s = 0; s < N_STATES; s++)
        for (int c = 0; c < N_CHARS; c++)
            transitions[s][c] = LexerState::Start;

    transitions[(int)LexerState::Start][(int)CharKind::Digit]  = LexerState::InNumber;
    transitions[(int)LexerState::Start][(int)CharKind::Letter] = LexerState::InName;
    transitions[(int)LexerState::Start][(int)CharKind::OpChar] = LexerState::InOperator;

    transitions[(int)LexerState::InNumber][(int)CharKind::Digit] = LexerState::InNumber;

    transitions[(int)LexerState::InName][(int)CharKind::Letter] = LexerState::InName;
    transitions[(int)LexerState::InName][(int)CharKind::Digit]  = LexerState::InName;

    transitions[(int)LexerState::InOperator][(int)CharKind::OpChar] = LexerState::InOperator;
}

void Lexer::loadKeywords() {
    kwMap["int"]    = TokenType::Keyword;
    kwMap["void"]   = TokenType::Keyword;
    kwMap["static"] = TokenType::Keyword;
    kwMap["return"] = TokenType::Keyword;
    kwMap["if"]     = TokenType::Keyword;
    kwMap["else"]   = TokenType::Keyword;
    kwMap["while"]  = TokenType::Keyword;
    kwMap["do"]     = TokenType::Keyword;
}

Lexer::CharKind Lexer::classify(char c) const {
    if (isdigit(c))                          return CharKind::Digit;
    if (isalpha(c) || c == '_')              return CharKind::Letter;
    if (isspace(c))                          return CharKind::Space;
    if (c=='+' || c=='-' || c=='*' ||
        c=='/' || c=='=' || c=='<' ||
        c=='>' || c=='!')                    return CharKind::OpChar;
    if (c=='(' || c==')' ||
        c=='{' || c=='}')                    return CharKind::Fence;
    if (c==';' || c==',')                    return CharKind::Sep;
    return CharKind::Other;
}

Token Lexer::buildToken() {
    if (buffer.empty())
        return Token(TokenType::EndOfExpr, "", line, column);

    switch (state) {
        case LexerState::InNumber:
            return Token(TokenType::Number, buffer, line, column);

        case LexerState::InName: {
            auto it = kwMap.find(buffer);
            if (it != kwMap.end())
                return Token(TokenType::Keyword, buffer, line, column);
            return Token(TokenType::Name, buffer, line, column);
        }

        case LexerState::InOperator: {
            if (buffer == "==" || buffer == "!=" ||
                buffer == "<"  || buffer == ">"  ||
                buffer == "<=" || buffer == ">=")
                return Token(TokenType::Comparison, buffer, line, column);
            if (buffer == "=")
                return Token(TokenType::Assignment, buffer, line, column);
            return Token(TokenType::Operator, buffer, line, column);
        }

        default:
            return Token(TokenType::Unknown, buffer, line, column);
    }
}

Token Lexer::singleChar(char c) {
    std::string s(1, c);
    switch (c) {
        case '(': return Token(TokenType::OpenParen,  s, line, column);
        case ')': return Token(TokenType::CloseParen, s, line, column);
        case '{': return Token(TokenType::OpenBrace,  s, line, column);
        case '}': return Token(TokenType::CloseBrace, s, line, column);
        case ';': return Token(TokenType::Semicolon,  s, line, column);
        case ',': return Token(TokenType::Comma,      s, line, column);
        default:  return Token(TokenType::Unknown,    s, line, column);
    }
}

Token Lexer::getNextToken() {
    if (!pending.empty()) {
        Token t = pending.back();
        pending.pop_back();
        return t;
    }

    buffer.clear();
    state = LexerState::Start;
    char c;

    while (src.get(c)) {
        column++;
        if (c == '\n') { line++; column = 0; }

        CharKind kind = classify(c);

        if (state == LexerState::Start) {
            if (kind == CharKind::Space)  continue;
            if (c == '/' && src.peek() == '/') {
                while (src.get(c) && c != '\n') {}
                line++; column = 0;
                continue;
            }

            if (kind == CharKind::Fence || kind == CharKind::Sep)
                return singleChar(c);

            LexerState next = transitions[(int)LexerState::Start][(int)kind];
            state   = next;
            buffer += c;
            continue;
        }

        if (state == LexerState::InOperator) {
            if (kind == CharKind::OpChar && buffer.size() == 1 && c == '=') {
                buffer += c;
            } else {
                src.unget(); column--;
            }
            Token tok = buildToken();
            buffer.clear();
            state = LexerState::Start;
            return tok;
        }

        LexerState next = transitions[(int)state][(int)kind];

        if (next == state) {
            buffer += c;
        } else {
            src.unget(); column--;
            Token tok = buildToken();
            buffer.clear();
            state = LexerState::Start;
            return tok;
        }
    }

    if (!buffer.empty()) {
        Token tok = buildToken();
        buffer.clear();
        return tok;
    }
    return Token(TokenType::EndOfExpr, "", line, column);
}

void Lexer::pushBack(const Token& tok) {
    pending.push_back(tok);
}

void Lexer::reset() {
    state = LexerState::Start;
    buffer.clear();
    pending.clear();
}
