#include <dplp_resolver.h>

#include <dplp_promise.h>
#include <gtest/gtest.h>

#include <string>

TEST(dplp_resolver, basic) {
  auto resolver = [](dplmrts::Invocable<int> fulfill,
                     dplmrts::Invocable<std::exception_ptr> reject) {
    fulfill(3);
  };
  EXPECT_EQ((dplp::Resolver<decltype(resolver), int>), true)
      << "incorrectly not detected as a resolver";
  EXPECT_EQ((dplp::Resolver<decltype(resolver), std::string>), false)
      << "incorrectly detected a resolver";
  EXPECT_EQ((dplp::Resolver<int, int>), false) << "int detected as a resolver";
}

namespace {
template <typename T> dplp::Promise<T> makePromise(dplp::Resolver<T> r) {
  return dplp::Promise<T>(r);
}
}

TEST(dplp_resolver, example) {
  dplp::Promise<int> r = makePromise<int>(
      [](dplmrts::Invocable<int> fulfill,
         dplmrts::Invocable<std::exception_ptr> reject) { fulfill(3); });
  bool fulfilled = false;
  r.then([&fulfilled](int i) {
    EXPECT_EQ(i, 3);
    fulfilled = true;
  });
  EXPECT_EQ(fulfilled, true);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

