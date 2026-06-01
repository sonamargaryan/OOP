#include "IRGenerator.h"
#include <iostream>
#include <stdexcept>

IRGenerator::IRGenerator() : tempCount(0), labelCount(0) {}


std::string IRGenerator::newTemp() {
    return "t" + std::to_string(tempCount++);
}

std::string IRGenerator::newLabel(const std::string& prefix) {
    return prefix + std::to_string(labelCount++);
}

void IRGenerator::emit(const TACInstruction& instr) {
    code.push_back(instr);
}

TACProgram IRGenerator::generate(ASTNode* node) {
    code.clear();
    tempCount  = 0;
    labelCount = 0;

    if (node->type == NodeType::ProgramNode)
        genProgram(static_cast<ProgramNode*>(node));
    else
        throw std::runtime_error("IRGenerator: expected ProgramNode");

    return code;
}

void IRGenerator::genProgram(ProgramNode* node) {
    for (auto& fn : node->functions)
        genFunction(static_cast<FunctionDefNode*>(fn.get()));
}

void IRGenerator::genFunction(FunctionDefNode* node) {
    emit({TACOp::FUNC_BEGIN, node->name});

    for (const Param& p : node->params)
        emit({TACOp::COPY, p.name, "arg_" + p.name});

    genBlock(static_cast<BlockNode*>(node->body.get()));

    emit({TACOp::FUNC_END, node->name});
}

void IRGenerator::genBlock(BlockNode* node) {
    for (auto& stmt : node->statements)
        genStatement(stmt.get());
}

void IRGenerator::genStatement(ASTNode* node) {
    if (!node) return;

    switch (node->type) {
        case NodeType::VarDeclNode:
            genVarDecl(static_cast<VarDeclNode*>(node));
            break;
        case NodeType::ReturnNode:
            genReturn(static_cast<ReturnNode*>(node));
            break;
        case NodeType::IfNode:
            genIf(static_cast<IfNode*>(node));
            break;
        case NodeType::WhileNode:
            genWhile(static_cast<WhileNode*>(node));
            break;
        case NodeType::DoWhileNode:
            genDoWhile(static_cast<DoWhileNode*>(node));
            break;
        case NodeType::AssignmentNode: {
            auto* a = static_cast<AssignmentNode*>(node);
            std::string src = genExpr(a->expr.get());
            emit({TACOp::COPY, a->varName, src});
            break;
        }
        default:
            genExpr(node); 
            break;
    }
}

void IRGenerator::genVarDecl(VarDeclNode* node) {
    if (node->initExpr) {
        std::string src = genExpr(node->initExpr.get());
        emit({TACOp::COPY, node->varName, src});
    } else {
        emit({TACOp::COPY, node->varName, "0"});
    }
}

void IRGenerator::genReturn(ReturnNode* node) {
    if (node->expr) {
        std::string src = genExpr(node->expr.get());
        emit({TACOp::RETURN, "", src});
    } else {
        emit({TACOp::RETURN, "", ""});
    }
}

void IRGenerator::genIf(IfNode* node) {
    std::string elseLabel = newLabel("if_else");
    std::string endLabel  = newLabel("if_end");

    std::string cond = genExpr(node->condition.get());
    emit({TACOp::IFFALSE, elseLabel, cond});

    genBlock(static_cast<BlockNode*>(node->thenBlock.get()));
    emit({TACOp::GOTO, endLabel});

    emit({TACOp::LABEL, elseLabel});
    if (node->elseBlock)
        genBlock(static_cast<BlockNode*>(node->elseBlock.get()));

    emit({TACOp::LABEL, endLabel});
}

void IRGenerator::genWhile(WhileNode* node) {
    std::string loopLabel = newLabel("while_L");
    std::string endLabel  = newLabel("while_end");

    emit({TACOp::LABEL, loopLabel});

    std::string cond = genExpr(node->condition.get());
    emit({TACOp::IFFALSE, endLabel, cond});

    genBlock(static_cast<BlockNode*>(node->body.get()));
    emit({TACOp::GOTO, loopLabel});

    emit({TACOp::LABEL, endLabel});
}

void IRGenerator::genDoWhile(DoWhileNode* node) {
    std::string loopLabel = newLabel("do_L");

    emit({TACOp::LABEL, loopLabel});

    genBlock(static_cast<BlockNode*>(node->body.get()));

    std::string cond = genExpr(node->condition.get());
    emit({TACOp::IFTRUE, loopLabel, cond});
}

TACOp IRGenerator::charToTACOp(char op) {
    switch (op) {
        case '+': return TACOp::ADD;
        case '-': return TACOp::SUB;
        case '*': return TACOp::MUL;
        case '/': return TACOp::DIV;
        case '<': return TACOp::LT;
        case '>': return TACOp::GT;
        case 'L': return TACOp::LE;
        case 'G': return TACOp::GE;
        case 'E': return TACOp::EQ;
        case 'N': return TACOp::NE;
        default:
            throw std::runtime_error(
                std::string("IRGenerator: unknown op '") + op + "'");
    }
}

std::string IRGenerator::genExpr(ASTNode* node) {
    if (!node) return "";

    switch (node->type) {

        case NodeType::NumberNode: {
            auto* n = static_cast<NumberNode*>(node);
            std::string t = newTemp();
            emit({TACOp::COPY, t, std::to_string(n->value)});
            return t;
        }

        case NodeType::VariableNode: {
            auto* v = static_cast<VariableNode*>(node);
            return v->name;
        }

        case NodeType::BinaryOpNode:
            return genBinaryOp(static_cast<BinaryOpNode*>(node));

        case NodeType::UnaryOpNode: {
            auto* u = static_cast<UnaryOpNode*>(node);
            std::string src = genExpr(u->operand.get());
            std::string t   = newTemp();
            emit({TACOp::NEG, t, src});
            return t;
        }

        case NodeType::AssignmentNode: {
            auto* a = static_cast<AssignmentNode*>(node);
            std::string src = genExpr(a->expr.get());
            emit({TACOp::COPY, a->varName, src});
            return a->varName;
        }

        case NodeType::FunctionCallNode: {
            auto* fc = static_cast<FunctionCallNode*>(node);

            for (auto& arg : fc->args) {
                std::string argVal = genExpr(arg.get());
                emit({TACOp::PARAM, "", argVal});
            }

            std::string t = newTemp();
            emit({TACOp::CALL, t, fc->funcName,
                  std::to_string(fc->args.size())});
            return t;
        }

        default:
            throw std::runtime_error("IRGenerator: unknown expression node");
    }
}

std::string IRGenerator::genBinaryOp(BinaryOpNode* node) {
    std::string left  = genExpr(node->left.get());
    std::string right = genExpr(node->right.get());
    std::string t     = newTemp();

    TACOp op = charToTACOp(node->op);
    emit({op, t, left, right});
    return t;
}

void IRGenerator::printTAC(const TACProgram& program) {
    std::cout << "\n═══ INTERMEDIATE CODE (TAC) ═══\n";
    for (const TACInstruction& instr : program) {
        if (instr.op == TACOp::LABEL     ||
            instr.op == TACOp::FUNC_BEGIN ||
            instr.op == TACOp::FUNC_END)
            std::cout << instr.toString() << "\n";
        else
            std::cout << "    " << instr.toString() << "\n";
    }
}