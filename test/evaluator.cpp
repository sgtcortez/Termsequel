#include "gtest/gtest.h"

#include <string>

#include "termsequel/evaluator/evaluator.hpp"

using namespace std;
using namespace termsequel;

TEST(Evaluator, EvaluateUnaryMinusExpression)
{
    const token::Token minus = token::Token(token::Type::UNARY_MINUS, token::Class::ARITHMETIC_OPERATOR, "-");

    {
        const string value = "3";
        const auto expected_result = "-3";

        const auto result = evaluator::evaluate(value, minus);
        ASSERT_EQ(result, expected_result);
    }
}

TEST(Evaluator, EvaluateUpperFunction)
{
    const token::Token upper = token::Token(token::Type::UPPER, token::Class::FUNCTION, "UPPER");
    {
        const string value = "somevalue";
        const auto expected_result = "SOMEVALUE";

        const auto result = evaluator::evaluate(value, upper);
        ASSERT_EQ(result, expected_result);
    }
    {
        const string value = "somev_alue";
        const auto expected_result = "SOMEV_ALUE";

        const auto result = evaluator::evaluate(value, upper);
        ASSERT_EQ(result, expected_result);
    }
    {
        const string value = "SOMEVALUE";
        const auto expected_result = "SOMEVALUE";

        const auto result = evaluator::evaluate(value, upper);
        ASSERT_EQ(result, expected_result);
    }
}

TEST(Evaluator, EvaluateLowerFunction)
{
    const token::Token lower = token::Token(token::Type::LOWER, token::Class::FUNCTION, "LOWER");
    {
        const string value = "somevalue";
        const auto expected_result = "somevalue";

        const auto result = evaluator::evaluate(value, lower);
        ASSERT_EQ(result, expected_result);
    }
    {
        const string value = "somev_alue";
        const auto expected_result = "somev_alue";

        const auto result = evaluator::evaluate(value, lower);
        ASSERT_EQ(result, expected_result);
    }
    {
        const string value = "SOMEVALUE";
        const auto expected_result = "somevalue";

        const auto result = evaluator::evaluate(value, lower);
        ASSERT_EQ(result, expected_result);
    }
}

TEST(Evaluator, EvaluateNotOperation)
{
    const token::Token not_op = token::Token(token::Type::NOT, token::Class::LOGICAL_OPERATOR, "NOT");
    {
        const string value = "1";
        const auto expected_result = "0";

        const auto result = evaluator::evaluate(value, not_op);
        ASSERT_EQ(result, expected_result);
    }
    {
        const string value = "0";
        const auto expected_result = "1";

        const auto result = evaluator::evaluate(value, not_op);
        ASSERT_EQ(result, expected_result);
    }
}

TEST(Evaluator, EvaluateAdditionBinaryExpresson)
{
    const token::Token addition = token::Token(token::Type::PLUS, token::Class::ARITHMETIC_OPERATOR, "+");
    {
        const string left = "5";
        const string right = "6";

        const auto result = evaluator::evaluate(left, addition, right);
        ASSERT_EQ(result, "11");
    }
    {
        const string left = "500";
        const string right = "300";

        const auto result = evaluator::evaluate(left, addition, right);
        ASSERT_EQ(result, "800");
    }
}

TEST(Evaluator, EvaluateMinusBinaryExpression)
{
    const token::Token minus = token::Token(token::Type::MINUS, token::Class::ARITHMETIC_OPERATOR, "-");
    {
        const string left = "5";
        const string right = "6";

        const auto result = evaluator::evaluate(left, minus, right);
        ASSERT_EQ(result, "-1");
    }
    {
        const string left = "500";
        const string right = "300";

        const auto result = evaluator::evaluate(left, minus, right);
        ASSERT_EQ(result, "200");
    }
}

TEST(Evaluator, EvaluateMultiplicationinaryExpression)
{
    const token::Token multiplication = token::Token(token::Type::MULTIPLICATION, token::Class::ARITHMETIC_OPERATOR, "*");
    {
        const string left = "5";
        const string right = "-6";

        const auto result = evaluator::evaluate(left, multiplication, right);
        ASSERT_EQ(result, "-30");
    }
    {
        const string left = "500";
        const string right = "300";

        const auto result = evaluator::evaluate(left, multiplication, right);
        ASSERT_EQ(result, "150000");
    }
}

TEST(Evaluator, EvaluateDivisionBinaryExpression)
{
    const token::Token division = token::Token(token::Type::DIVIDE, token::Class::ARITHMETIC_OPERATOR, "/");
    {
        const string left = "30";
        const string right = "-6";

        const auto result = evaluator::evaluate(left, division, right);
        ASSERT_EQ(result, "-5");
    }
    {
        const string left = "100";
        const string right = "2";

        const auto result = evaluator::evaluate(left, division, right);
        ASSERT_EQ(result, "50");
    }
}

TEST(Evaluator, EvaluateAndLogicalExpression)
{
    const token::Token and_op =  token::Token(token::Type::AND, token::Class::LOGICAL_OPERATOR, "AND");
    {
        const string left = "1";
        const string right = "1";

        const auto result = evaluator::evaluate(left, and_op, right);
        ASSERT_EQ(result, "1");
    }
    {
        const string left = "1";
        const string right = "0";

        const auto result = evaluator::evaluate(left, and_op, right);
        ASSERT_EQ(result, "0");
    }
    {
        const string left = "0";
        const string right = "1";

        const auto result = evaluator::evaluate(left, and_op, right);
        ASSERT_EQ(result, "0");
    }
    {
        const string left = "0";
        const string right = "0";

        const auto result = evaluator::evaluate(left, and_op, right);
        ASSERT_EQ(result, "0");
    }
}

TEST(Evaluator, EvaluateOrLogicalExpression)
{
    const token::Token or_op =  token::Token(token::Type::OR, token::Class::LOGICAL_OPERATOR, "OR");
    {
        const string left = "1";
        const string right = "1";

        const auto result = evaluator::evaluate(left, or_op, right);
        ASSERT_EQ(result, "1");
    }
    {
        const string left = "1";
        const string right = "0";

        const auto result = evaluator::evaluate(left, or_op, right);
        ASSERT_EQ(result, "1");
    }
    {
        const string left = "0";
        const string right = "1";

        const auto result = evaluator::evaluate(left, or_op, right);
        ASSERT_EQ(result, "1");
    }
    {
        const string left = "0";
        const string right = "0";

        const auto result = evaluator::evaluate(left, or_op, right);
        ASSERT_EQ(result, "0");
    }
}