#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

using namespace std;

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

bool isDeclaration(const string& line) {
    return line.find("int ") == 0 ||
        line.find("float ") == 0 ||
        line.find("double ") == 0 ||
        line.find("char ") == 0;
}

bool isScanf(const string& line) {
    return line.find("scanf") != string::npos;
}

bool isPrintf(const string& line) {
    return line.find("printf") != string::npos;
}

bool isAssignment(const string& line) {
    return line.find('=') != string::npos && !isScanf(line) && !isDeclaration(line);
}

bool isIf(const string& line) {
    return line.find("if") == 0;
}

bool isElse(const string& line) {
    return line.find("else") == 0;
}

bool isWhile(const string& line) {
    return line.find("while") == 0;
}

bool isFor(const string& line) {
    return line.find("for") == 0;
}

void handleDeclaration(const string& line) {
    string type;
    size_t spacePos = line.find(' ');
    if (spacePos == string::npos) return;

    type = line.substr(0, spacePos);
    string content = line.substr(spacePos + 1);

    if (content.back() == ';') content.pop_back();

    stringstream ss(content);
    string var;
    while (getline(ss, var, ',')) {
        var = trim(var);

        if (var.find('=') != string::npos) {
            size_t eq = var.find('=');
            string name = trim(var.substr(0, eq));
            string expr = trim(var.substr(eq + 1));

            stringstream exprStream(expr);
            string lhs, rhs;
            char op;

            exprStream >> lhs;
            if (exprStream >> op >> rhs) {
                string instr;
                switch (op) {
                case '+': instr = "ADD"; break;
                case '-': instr = "SUB"; break;
                case '*': instr = "MUL"; break;
                case '/': instr = "DIV"; break;
                default: instr = "???"; break;
                }
                cout << instr << " " << name << ", " << lhs << ", " << rhs << endl;
            }
            else {
                cout << "MOV " << name << ", " << lhs << endl;
            }
        }
        else {
            cout << "DECL " << var << endl;
        }
    }
}

void handleScanf(const string& line) {
    size_t amp = line.find('&');
    size_t end = line.find(')', amp);
    if (amp != string::npos && end != string::npos) {
        string var = line.substr(amp + 1, end - amp - 1);
        var = trim(var);
        cout << "IN " << var << endl;
    }
}

void handlePrintf(const string& line) {
    size_t comma = line.find(',');
    size_t end = line.find(')', comma);
    if (comma != string::npos && end != string::npos) {
        string var = line.substr(comma + 1, end - comma - 1);
        var = trim(var);
        cout << "OUT " << var << endl;
    }
}

void handleAssignment(const string& line) {
    string code = line;
    if (code.back() == ';') code.pop_back();
    stringstream ss(code);
    string lhs, eq, rhs1, rhs2;
    char op;

    ss >> lhs >> eq >> rhs1;
    if (!(ss >> op >> rhs2)) {
        cout << "MOV " << lhs << ", " << rhs1 << endl;
    }
    else {
        string instr;
        switch (op) {
        case '+': instr = "ADD"; break;
        case '-': instr = "SUB"; break;
        case '*': instr = "MUL"; break;
        case '/': instr = "DIV"; break;
        default: instr = "???"; break;
        }
        cout << instr << " " << lhs << ", " << rhs1 << ", " << rhs2 << endl;
    }
}

void handleIf(const string& line) {
    size_t start = line.find('(');
    size_t end = line.find(')', start);
    if (start == string::npos || end == string::npos) return;

    string condition = trim(line.substr(start + 1, end - start - 1));
    string lhs, rhs;
    string op;
    stringstream ss(condition);
    ss >> lhs >> op >> rhs;

    string instr;
    if (op == "<") instr = "IF_LT";
    else if (op == ">") instr = "IF_GT";
    else if (op == "==") instr = "IF_EQ";
    else if (op == "!=") instr = "IF_NE";
    else if (op == "<=") instr = "IF_LE";
    else if (op == ">=") instr = "IF_GE";
    else instr = "IF_???";

    cout << instr << " " << lhs << ", " << rhs << ", LABEL" << endl;
}

void handleWhile(const string& line) {
    cout << "LOOP_START:" << endl;
    handleIf(line); // Reuse the IF translator
    // Assume loop body will be inserted manually or with braces logic
}

void handleFor(const string& line) {
    // For: for(i=0; i<n; i++)
    size_t start = line.find('(');
    size_t end = line.find(')', start);
    if (start == string::npos || end == string::npos) return;

    string content = line.substr(start + 1, end - start - 1);
    stringstream ss(content);
    string init, cond, incr;
    getline(ss, init, ';');
    getline(ss, cond, ';');
    getline(ss, incr);

    cout << "// For Loop Initialization" << endl;
    handleAssignment(trim(init));
    cout << "LOOP_START:" << endl;
    handleIf("if(" + cond + ")"); // treat like if
    cout << "// Loop Body" << endl;
    cout << "// Increment" << endl;
    handleAssignment(trim(incr));
    cout << "JMP LOOP_START" << endl;
}

int main() {
    ios_base::sync_with_stdio(false); cin.tie(0);
#ifndef ONLINE_JUDGE
    freopen("input.txt", "r", stdin); freopen("output.txt", "w", stdout);
#endif

    string line;
    while (getline(cin, line)) {
        line = trim(line);
        if (line == "end") break;
        if (line.empty()) continue;

        if (line.find("#") == 0 || line.find("main") != string::npos || line == "{" || line == "}" || line.find("return") != string::npos)
            continue;

        if (isDeclaration(line)) handleDeclaration(line);
        else if (isScanf(line)) handleScanf(line);
        else if (isPrintf(line)) handlePrintf(line);
        else if (isAssignment(line)) handleAssignment(line);
        else if (isIf(line)) handleIf(line);
        else if (isElse(line)) cout << "ELSE BLOCK" << endl;
        else if (isWhile(line)) handleWhile(line);
        else if (isFor(line)) handleFor(line);
    }

    return 0;
}
