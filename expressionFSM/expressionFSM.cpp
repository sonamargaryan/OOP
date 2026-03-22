#include <stack>
#include <iostream>
#include <string>
#include <vector>
#include <stack>
using namespace std;

enum TokenType { 
    VARIABLE,NUMBER ,
    PLUS, MINUS,MUL,DIV,
    LPAREN,RPAREN,
    END_OF_INPUT
};

struct Token
{
    TokenType type;
    string  value;
};

struct Node{
    string value;
    Node *left;
    Node *right;

    Node(string v){
        value = v;
        left = NULL;
        right = NULL;
    }
};

bool isOperator(char c) {
    return (c == '+'  c == '-'  c == '*'  c == '/'  c == '(' || c == ')');
}

vector <string> tokens;
int pos = 0;

vector<string> Lexer(string exp){
    vector <string> s;
    int i = 0;
    while (i < exp.length()) {
        char c = exp[i];
        if (isspace(c)) {
            i++;
            continue;
        }
        if (isdigit(c))
        {
            string num;
            while(i < exp.length() && isdigit(exp[i])){
                num += exp[i];
                i++;
            }
            s.push_back(num);
            continue;
        }
        if (isalpha(c) || c == '_')
        {
            string var;
            while (i < exp.length() && (isalnum(exp[i]) || exp[i] == '_')) {
                var += exp[i];
                i++;
            }
            s.push_back(var);
            continue;          
        }
        if (isOperator(c)){
            string op(1, c); 
            s.push_back(op);
            i++;
            continue;
        }
    }
    return (s);
}

vector <Token> Tokenizer(vector<string> lexemes)
{
    vector <Token> tokens;
    for (int i=0; i < lexemes.size(); i++){
        string s = lexemes[i];

        if (isdigit(s[0]))
            tokens.push_back({NUMBER, s});
        else if (isalpha(s[0]) || s[0] == '_')
            tokens.push_back({VARIABLE, s});
        else if (s == "+")
            tokens.push_back({PLUS, s});
        else if (s == "-")
            tokens.push_back({MINUS, s});
        else if (s == "*")
            tokens.push_back({MUL, s});
        else if (s == "/")
            tokens.push_back({DIV, s});
        else if (s == "(")
            tokens.push_back({LPAREN, s});
        else if (s == ")")
            tokens.push_back({RPAREN, s});
    }
    return tokens;
}

int precedence(TokenType t)
{
    if (t == PLUS || t == MINUS)
        return 1;
    if (t == MUL || t == DIV) 
        return 2;
    return 0;
}

Node* create_node(Token op, stack<Node*>& st)
{
    Node* right = st.top();
        st.pop();
    Node* left = st.top();
        st.pop();
    Node* node = new Node(op.value);
    node->left = left;
    node->right = right;
    return node;
}
Node* parse(vector<Token> tokens) {
    stack<Node*> nodes;
    stack<Token> ops;

    for (int i = 0; i < tokens.size(); i++)
    {
        Token t = tokens[i];
        if (t.type == NUMBER || t.type == VARIABLE) {
            nodes.push(new Node(t.value));
        }
        else if (t.type == LPAREN) {
            ops.push(t);
        }
        else if (t.type == RPAREN) {
            while (!ops.empty() && ops.top().type != LPAREN) {
                nodes.push(create_node(ops.top(), nodes));
                ops.pop();
            }
            ops.pop();
        }
        else {
            while (!ops.empty() &&
                   precedence(ops.top().type) >= precedence(t.type)) {
                nodes.push(create_node(ops.top(), nodes));
                ops.pop();
            }
            ops.push(t);
        }
    }
    while (!ops.empty()) {
        nodes.push(create_node(ops.top(), nodes));
        ops.pop();
    }
    return nodes.top();
}

void print_tree(Node* root, int depth = 0) {
    if (!root) return;

    print_tree(root->right, depth + 1);

    for (int i = 0; i < depth; i++) cout << "   ";
    cout << root->value << endl;

    print_tree(root->left, depth + 1);
}

int main(){
    string s = "(5+7) * 8";
    vector<string> l = Lexer(s);
    vector<Token> t = Tokenizer(l);
    cout << "t: ";
    for (int i = 0; i < t.size(); i++)
    {
        cout << t[i].type << " : " << t[i].value << endl;
    }
    Node* root = parse(t);

    cout << "\nAST:\n";
    print_tree(root);

    return 0;
}
