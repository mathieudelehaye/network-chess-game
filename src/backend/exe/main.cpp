#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "parser/GameVisitor.h"

using namespace std;
using namespace antlr4;
namespace fs = std::filesystem;

int main() {
    cout << "Start parsing" << endl;

    fs::path filepath("/mnt/c/Users/mathi/source/repos/cpp/sucden-fin-chess/test/game/game_01");

    if (!fs::exists(filepath)) {
        cerr << "Error: File does not exist: " << filepath << endl;
        return 1;
    }

    ifstream stream(filepath);
    if (!stream.is_open()) {
        cerr << "Error: Cannot open file " << filepath << endl;
        return 1;
    }

    // Create ANTLR input stream from file
    ANTLRInputStream input(stream);

    // Create lexer (tokenizer)
    SimpleChessGameLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    // Create parser
    SimpleChessGameParser parser(&tokens);

    // Parse the file (starting from 'game' rule)
    tree::ParseTree *tree = parser.game();

    // Walk the parse tree with our custom listener
    GameVisitor visitor;
    visitor.visit(tree);

    cout << "\nParsed moves:" << endl;

    for (const auto &move : visitor.getMoves()) {
        cout << move.first << "-" << move.second << endl;
    }

    return 0;
}