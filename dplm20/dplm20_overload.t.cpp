#include <dplm20_overload.h>

#include <gtest/gtest.h>
#include <string>

namespace {
int test1(std::string) { return 0; }
int test2(int i) { return i; }
}

TEST(dplm20_overload, basic) {
  auto test = dplm20::overload(test1, test2);
  EXPECT_EQ(test("hello"), 0) << "The wrong overload was selected.";
  EXPECT_EQ(test(3), 3) << "The wrong overload was selected.";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
