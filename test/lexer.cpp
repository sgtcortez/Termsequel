// https://google.github.io/googletest/primer.html

#include "gtest/gtest.h"

#include <string>

#include "termsequel/lexer/lexer.hpp"

using namespace std;
using namespace termsequel;


TEST(Lexer, MustReturnNextTokensCorrectly)
{
    const string input = R"(SELECT NAME, 10 + 5 AS "expr" FROM "/home" where UPPER(LOWER(NAME)) = "Some Name" OR SIZE >= 1000 * 1000)";

    lexer::Lexer lexer(input);

    ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::COMMA, token::Class::SPECIAL, ","), lexer.next());
    ASSERT_EQ(token::Token(token::Type::NUMBER, token::Class::ENTRY, "10"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::PLUS, token::Class::ARITHMETIC_OPERATOR, "+"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::NUMBER, token::Class::ENTRY, "5"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::AS, token::Class::KEYWORD, "AS"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::STRING, token::Class::ENTRY, "expr"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::FROM, token::Class::KEYWORD, "FROM"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::STRING, token::Class::ENTRY, "/home"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::WHERE, token::Class::KEYWORD, "WHERE"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::UPPER, token::Class::FUNCTION, "UPPER"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::LEFT_PARENTHESIS, token::Class::SPECIAL, "("), lexer.next());
    ASSERT_EQ(token::Token(token::Type::LOWER, token::Class::FUNCTION, "LOWER"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::LEFT_PARENTHESIS, token::Class::SPECIAL, "("), lexer.next());
    ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::RIGHT_PARENTHESIS, token::Class::SPECIAL, ")"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::RIGHT_PARENTHESIS, token::Class::SPECIAL, ")"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::EQUAL, token::Class::RELATIONAL_OPERATOR, "="), lexer.next());
    ASSERT_EQ(token::Token(token::Type::STRING, token::Class::ENTRY, "Some Name"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::OR, token::Class::LOGICAL_OPERATOR, "OR"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "SIZE"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::GREATER_THAN_OR_EQUAL, token::Class::RELATIONAL_OPERATOR, ">="), lexer.next());
    ASSERT_EQ(token::Token(token::Type::NUMBER, token::Class::ENTRY, "1000"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::MULTIPLICATION, token::Class::ARITHMETIC_OPERATOR, "*"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::NUMBER, token::Class::ENTRY, "1000"), lexer.next());
    ASSERT_FALSE(lexer.has_next());
}

TEST(Lexer, MustIgnoreCaseForKeywordsAndIdentifiers)
{
    const string input = R"(sEleCt naME, 10 + 5 As "ExpR" FRom "/HomE")";

    lexer::Lexer lexer(input);

    ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::COMMA, token::Class::SPECIAL, ","), lexer.next());
    ASSERT_EQ(token::Token(token::Type::NUMBER, token::Class::ENTRY, "10"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::PLUS, token::Class::ARITHMETIC_OPERATOR, "+"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::NUMBER, token::Class::ENTRY, "5"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::AS, token::Class::KEYWORD, "AS"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::STRING, token::Class::ENTRY, "ExpR"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::FROM, token::Class::KEYWORD, "FROM"), lexer.next());
    ASSERT_EQ(token::Token(token::Type::STRING, token::Class::ENTRY, "/HomE"), lexer.next());
    ASSERT_FALSE(lexer.has_next());
}

TEST(Lexer, HasNextMustReturnForEmptyOrNullInputs)
{
    {
        lexer::Lexer lexer("");
        ASSERT_FALSE(lexer.has_next());
    }
    {
        lexer::Lexer lexer("     ");
        ASSERT_FALSE(lexer.has_next());
    }
}

TEST(Lexer, MustPeekNotConsumeFromStream)
{
    {
        const string input = "SELECT";
        lexer::Lexer lexer(input);
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.peek());
        ASSERT_TRUE(lexer.has_next());
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.next());
        ASSERT_FALSE(lexer.has_next());
    }
    {
        const string input = "SELECT NAME";
        lexer::Lexer lexer(input);
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.next());
        ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.peek());
        ASSERT_TRUE(lexer.has_next());
    }
}

TEST(Lexer, MustConsumePreviousConsumedToken)
{
    {
        const string input = "SELECT NAME";
        lexer::Lexer lexer(input);
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.next());
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.previous());
        ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.next());
        ASSERT_FALSE(lexer.has_next());
        ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.previous());
        ASSERT_FALSE(lexer.has_next());
    }
}

TEST(Lexer, MustPukeConsumedTokenAndRestoreItBackToTheStream)
{
    {
        const string input = "SELECT NAME";
        lexer::Lexer lexer(input);
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.next());
        ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.next());

        // ensure that we reached the end of the stream
        ASSERT_FALSE(lexer.has_next());

        // puke the NAME identifier token
        lexer.puke();
        ASSERT_EQ(token::Token(token::Type::IDENTIFIER, token::Class::ENTRY, "NAME"), lexer.peek());

        // puke the select keyword
        lexer.puke();
        ASSERT_EQ(token::Token(token::Type::SELECT, token::Class::KEYWORD, "SELECT"), lexer.peek());

        // check that still has tokens to be consumed
        ASSERT_TRUE(lexer.has_next());
    }
}

