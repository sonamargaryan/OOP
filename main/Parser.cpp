#include "Parser.h"
#include "Lexer.h"
#include <stdexcept>

Parser::Parser(Lexer& lex, SymbolTable& symTable)
    : lexer(lex), symbolTable(symTable), currentState(ParserState::Start)
{
    initializeTransitionMatrix();
}

void Parser::initializeTransitionMatrix() {
    for (int s = 0; s < STATE_COUNT; s++)
        for (int t = 0; t < TOKEN_TYPE_COUNT; t++)
            transitionMatrix[s][t] = ParserState::Error;

    transitionMatrix[0][1] = ParserState::Operand;    
    transitionMatrix[0][2] = ParserState::Operand;    
    transitionMatrix[0][3] = ParserState::Operator;   
    transitionMatrix[0][4] = ParserState::Operand;    

    transitionMatrix[1][3] = ParserState::Operator;   
    transitionMatrix[1][7] = ParserState::Operator;   
    transitionMatrix[1][6] = ParserState::Assignment; 
    transitionMatrix[1][5] = ParserState::Operand;   
    transitionMatrix[1][0] = ParserState::Start;     
    transitionMatrix[1][8] = ParserState::Operand;    
    transitionMatrix[1][9] = ParserState::Operand;    

    transitionMatrix[2][1] = ParserState::Operand;    
    transitionMatrix[2][2] = ParserState::Operand;    
    transitionMatrix[2][4] = ParserState::Operand;    
    transitionMatrix[2][3] = ParserState::Operator;   

    transitionMatrix[3][1] = ParserState::Operand;    
    transitionMatrix[3][2] = ParserState::Operand;    
    transitionMatrix[3][4] = ParserState::Operand;    
    transitionMatrix[3][3] = ParserState::Operator;   
}

int Parser::getTokenTypeIndex(TokenType type) {
    switch (type) {
        case TokenType::EndOfExpr:  return 0;
        case TokenType::Number:     return 1;
        case TokenType::Name:       return 2;
        case TokenType::Operator:   return 3;
        case TokenType::OpenParen:  return 4;
        case TokenType::CloseParen: return 5;
        case TokenType::Assignment: return 6;
        case TokenType::Comparison: return 7;
        case TokenType::Semicolon:  return 8;
        case TokenType::Comma:      return 9;
        default:                    return 0;
    }
}

int Parser::getOperatorPrecedence(char op) {
    if (op == 'u')              return 5;  
    if (op == '*' || op == '/') return 4;
    if (op == '+' || op == '-') return 3;
    
    if (op == '<' || op == '>' || op == 'G' || op == 'L' ||
        op == 'E' || op == 'N')             return 2;
    if (op == '=')              return 1;
    return 0;
}

static char encodeComparison(const std::string& s) {
    if (s == "<")  return '<';
    if (s == ">")  return '>';
    if (s == "<=") return 'L';
    if (s == ">=") return 'G';
    if (s == "==") return 'E';
    if (s == "!=") return 'N';
    return '?';
}

void Parser::applyOperator() {
    if (operatorStack.empty()) return;
    char op = operatorStack.top();
    operatorStack.pop();

    if (op == 'u') {
        if (nodeStack.empty())
            throw std::runtime_error("Parser: no operand for unary minus");
        auto operand = std::move(nodeStack.top());
        nodeStack.pop();
        nodeStack.push(std::make_unique<UnaryOpNode>('-', std::move(operand)));
        return;
    }

    if (nodeStack.size() < 2)
        throw std::runtime_error("Parser: not enough operands for '" + std::string(1, op) + "'");

    auto right = std::move(nodeStack.top()); nodeStack.pop();
    auto left  = std::move(nodeStack.top()); nodeStack.pop();

    if (op == '=') {
        if (left->type != NodeType::VariableNode)
            throw std::runtime_error("Parser: left side of '=' must be a variable");
        std::string varName = static_cast<VariableNode*>(left.get())->name;
        nodeStack.push(std::make_unique<AssignmentNode>(varName, std::move(right), symbolTable));
    } else {
        nodeStack.push(std::make_unique<BinaryOpNode>(op, std::move(left), std::move(right)));
    }
}

void Parser::processToken() {
    switch (currentToken.type) {

        case TokenType::Number: {
            int32_t val = static_cast<int32_t>(std::stol(currentToken.value));
            nodeStack.push(std::make_unique<NumberNode>(val));
            break;
        }

        case TokenType::Name: {
            if (!symbolTable.exists(currentToken.value))
                symbolTable.declare(currentToken.value);
            nodeStack.push(std::make_unique<VariableNode>(currentToken.value, symbolTable));
            break;
        }

        case TokenType::Operator: {
            char op = currentToken.value[0];

            bool isUnary = (currentState == ParserState::Start   ||
                            currentState == ParserState::Operator ||
                            currentState == ParserState::Assignment);
            if (isUnary && op == '-') {
                operatorStack.push('u');
                break;
            }

            while (!operatorStack.empty() && operatorStack.top() != '(' &&
                   getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op))
                applyOperator();
            operatorStack.push(op);
            break;
        }

        case TokenType::Comparison: {
            char op = encodeComparison(currentToken.value);
            while (!operatorStack.empty() && operatorStack.top() != '(' &&
                   getOperatorPrecedence(operatorStack.top()) >= getOperatorPrecedence(op))
                applyOperator();
            operatorStack.push(op);
            break;
        }

        case TokenType::Assignment: {
            while (!operatorStack.empty() && operatorStack.top() != '(')
                applyOperator();
            operatorStack.push('=');
            break;
        }

        case TokenType::OpenParen:
            operatorStack.push('(');
            break;

        case TokenType::CloseParen:
            while (!operatorStack.empty() && operatorStack.top() != '(')
                applyOperator();
            if (!operatorStack.empty() && operatorStack.top() == '(')
                operatorStack.pop();
            else
                throw std::runtime_error("Parser: mismatched parentheses");
            break;

        default:
            break;
    }
}

std::unique_ptr<ASTNode> Parser::parse(bool stopAtCloseParen) {
    reset();

    while (true) {
        Token tok = lexer.getNextToken();

        if (tok.type == TokenType::EndOfExpr) break;

        if (tok.type == TokenType::Semicolon) {
            lexer.pushBack(tok);
            break;
        }

        if (tok.type == TokenType::Keyword  ||
            tok.type == TokenType::OpenBrace ||
            tok.type == TokenType::CloseBrace) {
            lexer.pushBack(tok);
            break;
        }

        if (tok.type == TokenType::CloseParen) {
            if (stopAtCloseParen) {
                lexer.pushBack(tok);
                break;
            }
        }

        if (tok.type == TokenType::Comma) {
            lexer.pushBack(tok);
            break;
        }

        int idx = getTokenTypeIndex(tok.type);
        ParserState next = transitionMatrix[(int)currentState][idx];

        if (next == ParserState::Error)
            throw std::runtime_error(
                "Parser syntax error at line " + std::to_string(tok.line) +
                ": unexpected token '" + tok.value + "'");

        currentToken = tok;
        processToken();
        currentState = next;
    }

    while (!operatorStack.empty()) applyOperator();

    if (nodeStack.empty())    return nullptr;
    auto result = std::move(nodeStack.top());
    nodeStack.pop();
    return result;
}

void Parser::reset() {
    currentState = ParserState::Start;
    while (!nodeStack.empty())    nodeStack.pop();
    while (!operatorStack.empty()) operatorStack.pop();
}
