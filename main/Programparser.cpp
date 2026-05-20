#include "ProgramParser.h"
#include <stdexcept>

ProgramParser::ProgramParser(Lexer& lexer, SymbolTable& symTable)
    : lexer(lexer), symTable(symTable), exprParser(lexer, symTable) {}

Token ProgramParser::nextToken() {
    return lexer.getNextToken();
}

void ProgramParser::pushBack(const Token& tok) {
    lexer.pushBack(tok);
}

Token ProgramParser::expect(TokenType type, const std::string& errorMsg) {
    Token tok = nextToken();
    if (tok.type != type)
        throw std::runtime_error(
            "Line " + std::to_string(tok.line) + ": " +
            errorMsg + " (got '" + tok.value + "')");
    return tok;
}

bool ProgramParser::isKeyword(const Token& tok, const std::string& kw) const {
    return tok.type == TokenType::Keyword && tok.value == kw;
}

bool ProgramParser::isTypeKeyword(const Token& tok) const {
    return tok.type == TokenType::Keyword &&
           (tok.value == "int" || tok.value == "void");
}

std::unique_ptr<ASTNode> ProgramParser::parseProgram() {
    auto program = std::make_unique<ProgramNode>();

    while (true) {
        Token tok = nextToken();

        if (tok.type == TokenType::EndOfExpr) break;

        if (isTypeKeyword(tok)) {
            std::string returnType = tok.value;

            Token nameTok = expect(TokenType::Name,
                                   "Expected function name after '" + returnType + "'");
            std::string funcName = nameTok.value;

            Token next = nextToken();
            if (next.type == TokenType::OpenParen) {
                program->functions.push_back(
                    parseFunction(returnType, funcName));
            } else {
                throw std::runtime_error(
                    "Line " + std::to_string(next.line) +
                    ": Expected '(' after function name '" + funcName + "'");
            }
        } else {
            throw std::runtime_error(
                "Line " + std::to_string(tok.line) +
                ": Expected function definition, got '" + tok.value + "'");
        }
    }

    return program;
}

std::unique_ptr<ASTNode> ProgramParser::parseFunction(const std::string& returnType,
                                                       const std::string& name) {
    SymbolTable localSymTable;

    std::vector<Param> params = parseParams();

    for (const Param& p : params)
        localSymTable.declare(p.name);

    expect(TokenType::OpenBrace, "Expected '{' after function parameters");

    
    auto body = parseBlock();

    auto funcDef = std::make_unique<FunctionDefNode>(
        returnType, name, std::move(params), std::move(body));

    return funcDef;
}

std::vector<Param> ProgramParser::parseParams() {
    std::vector<Param> params;

    Token tok = nextToken();

    if (tok.type == TokenType::CloseParen)
        return params;

    pushBack(tok);

    while (true) {
        // Type — int
        Token typeTok = nextToken();
        if (!isTypeKeyword(typeTok))
            throw std::runtime_error(
                "Line " + std::to_string(typeTok.line) +
                ": Expected type in parameter list, got '" + typeTok.value + "'");

        Token nameTok = expect(TokenType::Name,
                               "Expected parameter name after '" + typeTok.value + "'");

        params.push_back(Param(typeTok.value, nameTok.value));

        Token sep = nextToken();
        if (sep.type == TokenType::CloseParen) break;
        if (sep.type != TokenType::Comma)
            throw std::runtime_error(
                "Line " + std::to_string(sep.line) +
                ": Expected ',' or ')' in parameter list");
    }

    return params;
}

std::unique_ptr<ASTNode> ProgramParser::parseBlock() {
    auto block = std::make_unique<BlockNode>();

    while (true) {
        Token tok = nextToken();

        if (tok.type == TokenType::CloseBrace) break;

        if (tok.type == TokenType::EndOfExpr)
            throw std::runtime_error("Unexpected end of file — missing '}'");

        pushBack(tok);

        block->statements.push_back(parseStatement());
    }

    return block;
}

std::unique_ptr<ASTNode> ProgramParser::parseStatement() {
    Token tok = nextToken();

    if (isKeyword(tok, "static")) {
        expect(TokenType::Keyword, "Expected 'int' after 'static'");
        return parseVarDecl(true);
    }

    if (isKeyword(tok, "int") || isKeyword(tok, "void"))
        return parseVarDecl(false);

    if (isKeyword(tok, "if"))
        return parseIf();

    if (isKeyword(tok, "while"))
        return parseWhile();

    if (isKeyword(tok, "do"))
        return parseDoWhile();

    if (isKeyword(tok, "return"))
        return parseReturn();

    pushBack(tok);
    auto expr = exprParser.parse();
    expect(TokenType::Semicolon, "Expected ';' after expression");
    return expr;
}

std::unique_ptr<ASTNode> ProgramParser::parseVarDecl(bool isStatic) {
    Token nameTok = expect(TokenType::Name, "Expected variable name");
    std::string varName = nameTok.value;

    Token tok = nextToken();

    std::unique_ptr<ASTNode> initExpr = nullptr;

    if (tok.type == TokenType::Assignment) {
        initExpr = exprParser.parse();
        expect(TokenType::Semicolon, "Expected ';' after variable declaration");
    } else if (tok.type == TokenType::Semicolon) {
        initExpr = nullptr;
    } else {
        throw std::runtime_error(
            "Line " + std::to_string(tok.line) +
            ": Expected '=' or ';' after variable name '" + varName + "'");
    }

    if (!symTable.exists(varName))
        symTable.declare(varName, 0, isStatic);

    return std::make_unique<VarDeclNode>(varName, isStatic, std::move(initExpr));
}

std::unique_ptr<ASTNode> ProgramParser::parseIf() {
    expect(TokenType::OpenParen, "Expected '(' after 'if'");

    auto condition = exprParser.parse(true);

    expect(TokenType::CloseParen, "Expected ')' after if condition");

    expect(TokenType::OpenBrace, "Expected '{' after if condition");

    auto thenBlock = parseBlock();

    std::unique_ptr<ASTNode> elseBlock = nullptr;

    Token tok = nextToken();
    if (isKeyword(tok, "else")) {
        expect(TokenType::OpenBrace, "Expected '{' after 'else'");
        elseBlock = parseBlock();
    } else {
        pushBack(tok);
    }

    return std::make_unique<IfNode>(
        std::move(condition),
        std::move(thenBlock),
        std::move(elseBlock));
}

std::unique_ptr<ASTNode> ProgramParser::parseWhile() {
    expect(TokenType::OpenParen, "Expected '(' after 'while'");

    auto condition = exprParser.parse(true);

    expect(TokenType::CloseParen, "Expected ')' after while condition");

    expect(TokenType::OpenBrace, "Expected '{' after while condition");

    auto body = parseBlock();

    return std::make_unique<WhileNode>(std::move(condition), std::move(body));
}

std::unique_ptr<ASTNode> ProgramParser::parseDoWhile() {
    expect(TokenType::OpenBrace, "Expected '{' after 'do'");

    auto body = parseBlock();

    Token whileTok = nextToken();
    if (!isKeyword(whileTok, "while"))
        throw std::runtime_error(
            "Line " + std::to_string(whileTok.line) +
            ": Expected 'while' after do block");

   expect(TokenType::OpenParen, "Expected '(' after 'while'");

    auto condition = exprParser.parse(true);

    expect(TokenType::CloseParen, "Expected ')' after do-while condition");

    expect(TokenType::Semicolon, "Expected ';' after do-while");

    return std::make_unique<DoWhileNode>(std::move(body), std::move(condition));
}

std::unique_ptr<ASTNode> ProgramParser::parseReturn() {
    Token tok = nextToken();
    if (tok.type == TokenType::Semicolon)
        return std::make_unique<ReturnNode>(nullptr);

    pushBack(tok);
    auto expr = exprParser.parse();
    expect(TokenType::Semicolon, "Expected ';' after return value");

    return std::make_unique<ReturnNode>(std::move(expr));
}
