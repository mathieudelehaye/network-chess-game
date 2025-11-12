#include <iostream>
#include <filesystem>
#include <fstream>
#include "antlr4-runtime.h"
#include "SimpleChessGameLexer.h"
#include "SimpleChessGameParser.h"
#include "SimpleChessGameBaseListener.h"

using namespace std;
namespace fs = std::filesystem;
using namespace antlr4;
using namespace chess;

// Custom listener to handle parsed moves
class GameListener : public SimpleChessGameBaseListener {
public:
    void enterStrike(SimpleChessGameParser::StrikeContext *ctx) override {
        string from = ctx->COORD(0)->getText();
        string to = ctx->COORD(1)->getText();
        cout << "Move: " << from << " -> " << to << endl;
    }
    
    void enterComment(SimpleChessGameParser::CommentContext *ctx) override {
        string comment = ctx->COMMENT()->getText();
        cout << "Comment: " << comment;
    }
};

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
    GameListener listener;
    tree::ParseTreeWalker::DEFAULT.walk(&listener, tree);
    
    cout << "\nParsing completed successfully!" << endl;

    return 0;
}