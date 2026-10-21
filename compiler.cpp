#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>
using namespace std;

struct Instruction {
    string op, arg1, arg2, result;
};

class TACGenerator {
private:
    string input;
    size_t pos;
    vector<Instruction> code;
    int tempCount;

    void skipSpaces() {
        while (pos < input.size() &&
               isspace((unsigned char)input[pos]))
            pos++;
    }

    bool match(char c) {
        skipSpaces();
        if (pos < input.size() && input[pos] == c) {
            pos++;
            return true;
        }
        return false;
    }

    string newTemp() {
        return "t" + to_string(++tempCount);
    }

    string parseFactor() {
        skipSpaces();

        if (match('(')) {
            string value = parseExpression();

            if (!match(')'))
                throw string("Missing closing parenthesis.");

            return value;
        }

        if (pos < input.size() &&
            (isalpha((unsigned char)input[pos]) ||
             input[pos] == '_')) {

            size_t start = pos++;

            while (pos < input.size() &&
                   (isalnum((unsigned char)input[pos]) ||
                    input[pos] == '_'))
                pos++;

            return input.substr(start, pos - start);
        }

        if (pos < input.size() &&
            isdigit((unsigned char)input[pos])) {

            size_t start = pos++;

            while (pos < input.size() &&
                   isdigit((unsigned char)input[pos]))
                pos++;

            if (pos < input.size() && input[pos] == '.') {
                pos++;

                while (pos < input.size() &&
                       isdigit((unsigned char)input[pos]))
                    pos++;
            }

            return input.substr(start, pos - start);
        }

        throw string("Invalid arithmetic expression.");
    }

    string parseTerm() {
        string left = parseFactor();

        while (true) {
            skipSpaces();

            if (pos < input.size() &&
                (input[pos] == '*' || input[pos] == '/')) {

                char op = input[pos++];
                string right = parseFactor();
                string temp = newTemp();

                code.push_back({
                    string(1, op), left, right, temp
                });

                left = temp;
            } else {
                break;
            }
        }

        return left;
    }

    string parseExpression() {
        string left = parseTerm();

        while (true) {
            skipSpaces();

            if (pos < input.size() &&
                (input[pos] == '+' || input[pos] == '-')) {

                char op = input[pos++];
                string right = parseTerm();
                string temp = newTemp();

                code.push_back({
                    string(1, op), left, right, temp
                });

                left = temp;
            } else {
                break;
            }
        }

        return left;
    }

public:
    explicit TACGenerator(const string& expr)
        : input(expr), pos(0), tempCount(0) {}

    bool generate(string& error) {
        try {
            string result = parseExpression();
            skipSpaces();

            if (pos != input.size()) {
                error = "Unexpected character '" +
                        string(1, input[pos]) +
                        "' at position " +
                        to_string(pos + 1) + ".";
                return false;
            }

            code.push_back({"=", result, "", "result"});
            return true;

        } catch (const string& message) {
            error = message;
            return false;
        }
    }

    const vector<Instruction>& getCode() const {
        return code;
    }
};

void display(const vector<Instruction>& code) {
    cout << "\nTHREE ADDRESS CODE\n";

    for (const auto& item : code) {
        if (item.op == "=") {
            cout << item.result << " = "
                 << item.arg1 << "\n";
        } else {
            cout << item.result << " = "
                 << item.arg1 << " "
                 << item.op << " "
                 << item.arg2 << "\n";
        }
    }

    cout << "\nQUADRUPLES\n";

    cout << left << setw(10) << "Operator"
         << setw(12) << "Arg1"
         << setw(12) << "Arg2"
         << setw(12) << "Result" << "\n";

    for (const auto& item : code) {
        cout << left << setw(10) << item.op
             << setw(12) << item.arg1
             << setw(12)
             << (item.arg2.empty() ? "-" : item.arg2)
             << setw(12) << item.result << "\n";
    }

    cout << "\nTRIPLES\n";

    cout << left << setw(8) << "Index"
         << setw(12) << "Operator"
         << setw(12) << "Arg1"
         << setw(12) << "Arg2" << "\n";

    for (size_t i = 0; i < code.size(); i++) {
        string a1 = code[i].arg1;
        string a2 = code[i].arg2.empty()
                        ? "-" : code[i].arg2;

        for (size_t j = 0; j < i; j++) {
            if (code[j].result == a1)
                a1 = "(" + to_string(j) + ")";

            if (code[j].result == a2)
                a2 = "(" + to_string(j) + ")";
        }

        cout << left << setw(8) << i
             << setw(12) << code[i].op
             << setw(12) << a1
             << setw(12) << a2 << "\n";
    }

    cout << "\nINDIRECT TRIPLES (Pointer Order)\n";
    cout << "Pointer\tTriple Index\n";

    for (size_t i = 0; i < code.size(); i++) {
        cout << "p" << i << "\t" << i << "\n";
    }
}

int main() {
    string expression;

    cout << "THREE ADDRESS CODE GENERATOR\n";
    cout << "Enter an arithmetic expression: ";

    getline(cin, expression);

    if (expression.empty()) {
        cout << "\nError: Expression cannot be empty.\n";
        return 1;
    }

    TACGenerator generator(expression);
    string error;

    if (!generator.generate(error)) {
        cout << "\nSyntax Error: " << error << "\n";
        return 1;
    }

    display(generator.getCode());

    return 0;
}
