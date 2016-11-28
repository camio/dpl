#include <dplp_anypromise.h>

#include <dplp_promise.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(dplp_anypromise, basic) {
  EXPECT_EQ((dplp::AnyPromise<dplp::promise<int>>), true)
      << "promise<int> not detected as a promise";
  EXPECT_EQ((dplp::AnyPromise<dplp::promise<int, std::string>>), true)
      << "promise<int,string> not detected as a promise";
  EXPECT_EQ((dplp::AnyPromise<int>), false) << "int detected as a promise";
}

namespace {
template <typename T> concept bool Container = true;

template <Container C>
requires !dplp::AnyPromise<typename C::value_type> int f(C c) {
  return 0;
}

template <Container C>
requires dplp::AnyPromise<typename C::value_type> int f(C c) {
  return 1;
}
}

TEST(dplp_anypromise, example) {
  EXPECT_EQ(f(std::vector<int>()), 0) << "The wrong overload was selected.";
  EXPECT_EQ(f(std::vector<dplp::promise<int>>()), 1)
      << "The wrong overload was selected.";
  EXPECT_EQ(f(std::vector<dplp::promise<int, char>>()), 1)
      << "The wrong overload was selected.";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
