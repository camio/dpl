#include <dplmrts_invocablearchetype.h>

#include <gtest/gtest.h>

#include <functional> // std::function
#include <string>

TEST(dplmrts_invocable, basic) {
  EXPECT_EQ((dplmrts::Invocable<dplmrts::InvocableArchetype<int>, int>), true)
      << "Archetype doesn't match concept";
  EXPECT_EQ((dplmrts::Invocable<dplmrts::InvocableArchetype<std::string>, int>),
            false)
      << "Archetype doesn't match concept";
  EXPECT_EQ((dplmrts::Invocable<int, int>), false)
      << "Archetype doesn't match concept";
  EXPECT_EQ((dplmrts::Invocable<dplmrts::InvocableArchetype<int, std::string>,
                                int, std::string>),
            true)
      << "Archetype doesn't match concept";
}

inline int foo(auto value) { return 0; }

inline int foo(dplmrts::Invocable<dplmrts::InvocableArchetype<int>> invocable) {
  return 1;
}

TEST(dplmrts_invocable, example) {
  EXPECT_EQ(foo(0), 0) << "The wrong overload was selected.";
  EXPECT_EQ(foo(std::function<void(std::function<void(int)>)>()), 1)
      << "The wrong overload was selected.";
  EXPECT_EQ(foo(std::function<void(std::function<void(std::string)>)>()), 0)
      << "The wrong overload was selected.";
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
