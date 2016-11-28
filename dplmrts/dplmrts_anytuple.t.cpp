#include <dplmrts_anytuple.h>

#include <gtest/gtest.h>

#include <tuple>
#include <vector>

TEST(dplmrts_anytuple, basic) {
  EXPECT_EQ((dplmrts::AnyTuple<std::tuple<int>>), true)
      << "tuple<int> not detected as a tuple";
  EXPECT_EQ((dplmrts::AnyTuple<std::tuple<int, std::string>>), true)
      << "tuple<int,string> not detected as a tuple";
  EXPECT_EQ((dplmrts::AnyTuple<int>), false) << "int detected as a tuple";
}

namespace {
template <typename T> concept bool Container = true;

template <Container C>
requires !dplmrts::AnyTuple<typename C::value_type> int f(C c) {
  return 0;
}

template <Container C>
requires dplmrts::AnyTuple<typename C::value_type> int f(C c) {
  return 1;
}
}

TEST(dplmrts_anytuple, example) {
  EXPECT_EQ(f(std::vector<int>()), 0) << "The wrong overload was selected.";
  EXPECT_EQ(f(std::vector<std::tuple<int>>()), 1)
      << "The wrong overload was selected.";
  EXPECT_EQ(f(std::vector<std::tuple<int, char>>()), 1)
      << "The wrong overload was selected.";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
