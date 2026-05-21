#pragma once
#include "SymbolTable.h"
#include <memory>
#include <string>
#include <vector>

enum class NodeType {
    NumberNode,
    VariableNode,
    BinaryOpNode,
    UnaryOpNode,
    AssignmentNode,
    FunctionCallNode,
    VarDeclNode,
    ReturnNode,
    IfNode,
    WhileNode,
    DoWhileNode,
    BlockNode,
    FunctionDefNode,
    ProgramNode
};

class ASTNode {
public:
    NodeType type;

    explicit ASTNode(NodeType t);
    virtual ~ASTNode() = default;
};

class NumberNode : public ASTNode {
public:
    int32_t value;
    explicit NumberNode(int32_t v);
};

class VariableNode : public ASTNode {
public:
    std::string  name;
    SymbolTable& symTable;

    VariableNode(const std::string& n, SymbolTable& st);
};

class BinaryOpNode : public ASTNode {
public:
    char                     op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOpNode(char o,
                 std::unique_ptr<ASTNode> l,
                 std::unique_ptr<ASTNode> r);
};

class UnaryOpNode : public ASTNode {
public:
    char                     op;
    std::unique_ptr<ASTNode> operand;

    UnaryOpNode(char o, std::unique_ptr<ASTNode> operand);
};

class AssignmentNode : public ASTNode {
public:
    std::string              varName;
    std::unique_ptr<ASTNode> expr;
    SymbolTable&             symTable;

    AssignmentNode(const std::string& name,
                   std::unique_ptr<ASTNode> e,
                   SymbolTable& st);
};

class FunctionCallNode : public ASTNode {
public:
    std::string                           funcName;
    std::vector<std::unique_ptr<ASTNode>> args;

    explicit FunctionCallNode(const std::string& name);
};

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;

    BlockNode();
};

class VarDeclNode : public ASTNode {
public:
    std::string              varName;
    bool                     isStatic;
    std::unique_ptr<ASTNode> initExpr;

    VarDeclNode(const std::string& name, bool stat, std::unique_ptr<ASTNode> init);
};

class ReturnNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expr;

    explicit ReturnNode(std::unique_ptr<ASTNode> e);
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> thenBlock;
    std::unique_ptr<ASTNode> elseBlock;

    IfNode(std::unique_ptr<ASTNode> cond,
           std::unique_ptr<ASTNode> thenB,
           std::unique_ptr<ASTNode> elseB);
};

class WhileNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    WhileNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> b);
};

class DoWhileNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> body;
    std::unique_ptr<ASTNode> condition;

    DoWhileNode(std::unique_ptr<ASTNode> b, std::unique_ptr<ASTNode> cond);
};


class Param {
public:
    std::string type;
    std::string name;

    Param(const std::string& t, const std::string& n);
};

class FunctionDefNode : public ASTNode {
public:
    std::string              returnType;
    std::string              name;
    std::vector<Param>       params;
    std::unique_ptr<ASTNode> body;

    FunctionDefNode(const std::string& ret,
                    const std::string& n,
                    std::vector<Param> p,
                    std::unique_ptr<ASTNode> b);
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> functions;

    ProgramNode();
};