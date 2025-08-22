#include "termsequel/parser/node.hpp"

#include "gtest/gtest.h"

#include <string>

#include "termsequel/lexer/lexer.hpp"

using namespace std;
using namespace termsequel;

TEST(TableNode, GetTypeMustReturnTable)
{
    const parser::TableNode node("mytb");
    ASSERT_EQ(node.get_type(), parser::NodeType::TABLE);
}

TEST(TableNode, SerializeMustReturnDataCorrectly)
{
    const parser::TableNode node("MY TABLE");
    ASSERT_EQ(node.serialize(), "TABLE(MY TABLE)");
}
