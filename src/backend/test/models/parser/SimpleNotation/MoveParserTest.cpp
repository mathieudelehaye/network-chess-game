#include <gtest/gtest.h>
#include "MoveParser.hpp"

class MoveParserTest : public ::testing::Test {
protected:
    void SetUp() override {
        parser = std::make_unique<MoveParser>();
    }

    std::unique_ptr<MoveParser> parser;
};

TEST_F(MoveParserTest, ParseValidMove) {
    auto result = parser->parse("e2-e4");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->from, "e2");
    EXPECT_EQ(result->to, "e4");
}

TEST_F(MoveParserTest, ParseMoveWithSpaces) {
    auto result = parser->parse("e2 - e4");
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->from, "e2");
    EXPECT_EQ(result->to, "e4");
}

TEST_F(MoveParserTest, ParseInvalidFormat) {
    auto result = parser->parse("invalid");
    
    EXPECT_FALSE(result.has_value());
}

TEST_F(MoveParserTest, ParseComment) {
    auto result = parser->parse("// This is a comment");
    
    EXPECT_FALSE(result.has_value());
}

TEST_F(MoveParserTest, ParseGameFromSimpleNotationFile) {
    std::string game_data = R"(
// This is a comment on e2-e4
e2-e4
e7-e5
// Another comment
g1-f3)";
    
    auto result = parser->parseGame(game_data);
    
    EXPECT_EQ(result.size(), 3);
    
    EXPECT_EQ(result[0].from, "e2");
    EXPECT_EQ(result[0].to, "e4");
    
    EXPECT_EQ(result[1].from, "e7");
    EXPECT_EQ(result[1].to, "e5");
    
    EXPECT_EQ(result[2].from, "g1");
    EXPECT_EQ(result[2].to, "f3");
}