#include <akt/stack.h>

#include <gtest/gtest.h>

using namespace akt;

class StackTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    data_stack.push(123);
    data_stack.push(456);
    data_stack.push(789);

    full_stack.push(123);
    full_stack.push(456);
    full_stack.push(789);
  }

  virtual void TearDown() {
  }

  Stack<int, 3> empty_stack;
  Stack<int, 3> full_stack;
  Stack<int, 5> data_stack;
};

TEST_F(StackTest, TestEmpty) {
  EXPECT_TRUE(empty_stack.depth() == 0);
  EXPECT_TRUE(empty_stack.empty());
  EXPECT_FALSE(data_stack.empty());
}

TEST_F(StackTest, TestFull) {
  EXPECT_TRUE(full_stack.depth() == 3);
  EXPECT_FALSE(empty_stack.full());
  EXPECT_TRUE(full_stack.full());
}

TEST_F(StackTest, TestReset) {
  EXPECT_TRUE(empty_stack.empty());
  empty_stack.reset();
  EXPECT_TRUE(empty_stack.empty());
  EXPECT_TRUE(empty_stack.depth() == 0);

  EXPECT_FALSE(data_stack.empty());
  data_stack.reset();
  EXPECT_TRUE(data_stack.empty());
  EXPECT_TRUE(data_stack.depth() == 0);
}

TEST_F(StackTest, TestSpaceRemaining) {
  EXPECT_EQ(3, empty_stack.space_remaining());
  EXPECT_EQ(2, data_stack.space_remaining());
  EXPECT_EQ(0, full_stack.space_remaining());
}

TEST_F(StackTest, TestPushPop) {
  int dummy;

  EXPECT_TRUE(data_stack.depth() == 3);
  ASSERT_TRUE(data_stack.pop(dummy));
  ASSERT_EQ(789, dummy);
  EXPECT_TRUE(data_stack.depth() == 2);
  ASSERT_TRUE(data_stack.pop(dummy));
  ASSERT_EQ(456, dummy);
  EXPECT_TRUE(data_stack.depth() == 1);
  ASSERT_TRUE(data_stack.pop(dummy));
  ASSERT_EQ(123, dummy);
  EXPECT_TRUE(data_stack.depth() == 0);

  ASSERT_TRUE(data_stack.empty());
  ASSERT_EQ(5, data_stack.space_remaining());
  ASSERT_FALSE(data_stack.pop(dummy));
}
