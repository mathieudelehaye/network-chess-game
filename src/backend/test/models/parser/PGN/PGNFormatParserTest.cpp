#include <gtest/gtest.h>
#include "PGNFormatParser.hpp"

class PGNFormatParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<PGNFormatParser>();
    }

    std::unique_ptr<PGNFormatParser> parser;
};

TEST_F(PGNFormatParserTest, ParseSinglePawnMove) {
    auto result = parser->parseMove("e4");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->notation, "e4");
    EXPECT_TRUE(result->is_san);
    EXPECT_EQ(result->from, "");
    EXPECT_EQ(result->to, "");
}

TEST_F(PGNFormatParserTest, ParseKnightMove) {
    auto result = parser->parseMove("Nf3");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->notation, "Nf3");
    EXPECT_TRUE(result->is_san);
}

TEST_F(PGNFormatParserTest, ParseCastlingKingside) {
    auto result = parser->parseMove("O-O");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->notation, "O-O");
    EXPECT_TRUE(result->is_san);
}

TEST_F(PGNFormatParserTest, ParseInvalidMove) {
    auto result = parser->parseMove("invalid123");
    EXPECT_FALSE(result.has_value());
}

TEST_F(PGNFormatParserTest, ParseGameFromPGNFile) {
    std::string pgn_game = R"([Event "Tactical Victory"]
[Site "Local"]
[Date "2025.11.23"]
[Round "1"]
[White "Hero"]
[Black "Opponent"]
[Result "1-0"]

1. e4 e5 2. Nf3 Nc6)";
    
    auto result = parser->parseGame(pgn_game);
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 4); 
    
    EXPECT_EQ((*result)[0].notation, "e4");
    EXPECT_TRUE((*result)[0].is_san);
    
    EXPECT_EQ((*result)[1].notation, "e5");
    EXPECT_TRUE((*result)[1].is_san);
    
    EXPECT_EQ((*result)[2].notation, "Nf3");
    EXPECT_TRUE((*result)[2].is_san);
    
    EXPECT_EQ((*result)[3].notation, "Nc6");
    EXPECT_TRUE((*result)[3].is_san);
}