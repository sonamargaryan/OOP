#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "ProgramParser.h"

void printAST(ASTNode* node, int depth = 0) {
    if (!node) return;
    std::string indent(depth * 2, ' ');
    switch (node->type) {
        case NodeType::ProgramNode: {
            std::cout << indent << "PROGRAM\n";
            auto* p = static_cast<ProgramNode*>(node);
            for (auto& fn : p->functions) printAST(fn.get(), depth + 1);
            break;
        }
        case NodeType::FunctionDefNode: {
            auto* f = static_cast<FunctionDefNode*>(node);
            std::cout << indent << "FUNCTION " << f->returnType << " " << f->name << "(";
            for (int i = 0; i < (int)f->params.size(); i++) {
                if (i) std::cout << ", ";
                std::cout << f->params[i].type << " " << f->params[i].name;
            }
            std::cout << ")\n";
            printAST(f->body.get(), depth + 1);
            break;
        }
        case NodeType::BlockNode: {
            std::cout << indent << "BLOCK\n";
            auto* b = static_cast<BlockNode*>(node);
            for (auto& s : b->statements) printAST(s.get(), depth + 1);
            break;
        }
        case NodeType::VarDeclNode: {
            auto* v = static_cast<VarDeclNode*>(node);
            std::cout << indent << "VAR_DECL " << (v->isStatic ? "static " : "") << v->varName << "\n";
            if (v->initExpr) printAST(v->initExpr.get(), depth + 1);
            break;
        }
        case NodeType::ReturnNode: {
            std::cout << indent << "RETURN\n";
            auto* r = static_cast<ReturnNode*>(node);
            if (r->expr) printAST(r->expr.get(), depth + 1);
            break;
        }
        case NodeType::IfNode: {
            auto* i = static_cast<IfNode*>(node);
            std::cout << indent << "IF\n";
            std::cout << indent << "  CONDITION:\n"; printAST(i->condition.get(), depth + 2);
            std::cout << indent << "  THEN:\n";      printAST(i->thenBlock.get(), depth + 2);
            if (i->elseBlock) { std::cout << indent << "  ELSE:\n"; printAST(i->elseBlock.get(), depth + 2); }
            break;
        }
        case NodeType::WhileNode: {
            auto* w = static_cast<WhileNode*>(node);
            std::cout << indent << "WHILE\n";
            std::cout << indent << "  CONDITION:\n"; printAST(w->condition.get(), depth + 2);
            std::cout << indent << "  BODY:\n";      printAST(w->body.get(), depth + 2);
            break;
        }
        case NodeType::DoWhileNode: {
            auto* dw = static_cast<DoWhileNode*>(node);
            std::cout << indent << "DO_WHILE\n";
            std::cout << indent << "  BODY:\n";      printAST(dw->body.get(), depth + 2);
            std::cout << indent << "  CONDITION:\n"; printAST(dw->condition.get(), depth + 2);
            break;
        }
        case NodeType::BinaryOpNode: {
            auto* b = static_cast<BinaryOpNode*>(node);
            std::cout << indent << "BINARY_OP '" << b->op << "'\n";
            printAST(b->left.get(), depth + 1);
            printAST(b->right.get(), depth + 1);
            break;
        }
        case NodeType::AssignmentNode: {
            auto* a = static_cast<AssignmentNode*>(node);
            std::cout << indent << "ASSIGN " << a->varName << "\n";
            printAST(a->expr.get(), depth + 1);
            break;
        }
        case NodeType::NumberNode: {
            auto* n = static_cast<NumberNode*>(node);
            std::cout << indent << "NUMBER " << n->value << "\n";
            break;
        }
        case NodeType::VariableNode: {
            auto* v = static_cast<VariableNode*>(node);
            std::cout << indent << "VAR " << v->name << "\n";
            break;
        }
        default:
            std::cout << indent << "NODE\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) { std::cerr << "Usage: ./compiler program.mylang\n"; return 1; }

    std::ifstream file(argv[1]);
    if (!file.is_open()) { std::cerr << "Error: cannot open '" << argv[1] << "'\n"; return 1; }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::cout << "═══ SOURCE CODE ═══\n" << source << "\n";

    try {
        std::istringstream ss(source);
        Lexer         lexer(ss);
        SymbolTable   symTable;
        ProgramParser parser(lexer, symTable);
        auto ast = parser.parseProgram();
        std::cout << "═══ AST ═══\n";
        printAST(ast.get());
        std::cout << "\nParsing successful!\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
