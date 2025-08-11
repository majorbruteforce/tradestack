#include <gtest/gtest.h>

namespace
{
int GetThree()
{
    return 3;
}
}  // namespace

TEST(TestingTests, IsThree)
{
    EXPECT_EQ(GetThree(), 3);
}

TEST(TestingTests, IfNotThree)
{
    ASSERT_EQ(GetThree(), 3) << "Oh no, a mistake!";
    EXPECT_FLOAT_EQ(23.23F, 23.23F);
}