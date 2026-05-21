#include "ASTNode.h"

ASTNode::ASTNode(NodeType t) : type(t) {}

NumberNode::NumberNode(int32_t v)
    : ASTNode(NodeType::NumberNode), value(v) {}

VariableNode::VariableNode(const std::string& n, SymbolTable& st)
    : ASTNode(NodeType::VariableNode), name(n), symTable(st) {
    if (!st.exists(n))
        st.declare(n);
}

BinaryOpNode::BinaryOpNode(char o,
                           std::unique_ptr<ASTNode> l,
                           std::unique_ptr<ASTNode> r)
    : ASTNode(NodeType::BinaryOpNode),
      op(o), left(std::move(l)), right(std::move(r)) {}

UnaryOpNode::UnaryOpNode(char o, std::unique_ptr<ASTNode> operand)
    : ASTNode(NodeType::UnaryOpNode),
      op(o), operand(std::move(operand)) {}

AssignmentNode::AssignmentNode(const std::string& name,
                               std::unique_ptr<ASTNode> e,
                               SymbolTable& st)
    : ASTNode(NodeType::AssignmentNode),
      varName(name), expr(std::move(e)), symTable(st) {}

FunctionCallNode::FunctionCallNode(const std::string& name)
    : ASTNode(NodeType::FunctionCallNode), funcName(name) {}

BlockNode::BlockNode()
    : ASTNode(NodeType::BlockNode) {}

VarDeclNode::VarDeclNode(const std::string& name, bool stat,
                         std::unique_ptr<ASTNode> init)
    : ASTNode(NodeType::VarDeclNode),
      varName(name), isStatic(stat), initExpr(std::move(init)) {}

ReturnNode::ReturnNode(std::unique_ptr<ASTNode> e)
    : ASTNode(NodeType::ReturnNode), expr(std::move(e)) {}

IfNode::IfNode(std::unique_ptr<ASTNode> cond,
               std::unique_ptr<ASTNode> thenB,
               std::unique_ptr<ASTNode> elseB)
    : ASTNode(NodeType::IfNode),
      condition(std::move(cond)),
      thenBlock(std::move(thenB)),
      elseBlock(std::move(elseB)) {}

WhileNode::WhileNode(std::unique_ptr<ASTNode> cond,
                     std::unique_ptr<ASTNode> b)
    : ASTNode(NodeType::WhileNode),
      condition(std::move(cond)), body(std::move(b)) {}

DoWhileNode::DoWhileNode(std::unique_ptr<ASTNode> b,
                         std::unique_ptr<ASTNode> cond)
    : ASTNode(NodeType::DoWhileNode),
      body(std::move(b)), condition(std::move(cond)) {}

Param::Param(const std::string& t, const std::string& n)
    : type(t), name(n) {}

FunctionDefNode::FunctionDefNode(const std::string& ret,
                                 const std::string& n,
                                 std::vector<Param> p,
                                 std::unique_ptr<ASTNode> b)
    : ASTNode(NodeType::FunctionDefNode),
      returnType(ret), name(n),
      params(std::move(p)), body(std::move(b)) {}

ProgramNode::ProgramNode()
    : ASTNode(NodeType::ProgramNode) {}