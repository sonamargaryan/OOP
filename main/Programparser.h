#pragma once
#include "Lexer.h"
#include "Parser.h"
#include "ASTNode.h"
#include "SymbolTable.h"
#include <memory>
#include <vector>

class ProgramParser {
public:
    ProgramParser(Lexer& lexer, SymbolTable& symTable);

    std::unique_ptr<ASTNode> parseProgram();

private:
    Lexer&        lexer;
    SymbolTable&  symTable;
    Parser        exprParser;  

    std::unique_ptr<ASTNode> parseFunction(const std::string& returnType,
                                           const std::string& name);

    std::vector<Param> parseParams();

    std::unique_ptr<ASTNode> parseBlock();

    std::unique_ptr<ASTNode> parseStatement();

    std::unique_ptr<ASTNode> parseVarDecl(bool isStatic);

    std::unique_ptr<ASTNode> parseIf();

    std::unique_ptr<ASTNode> parseWhile();

    std::unique_ptr<ASTNode> parseDoWhile();

    std::unique_ptr<ASTNode> parseReturn();


    Token nextToken();

    void  pushBack(const Token& tok);

    Token expect(TokenType type, const std::string& errorMsg);

    bool  isKeyword(const Token& tok, const std::string& kw) const;

    bool  isTypeKeyword(const Token& tok) const;
};
