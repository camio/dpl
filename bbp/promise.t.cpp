#include <dpl/bbp/promise.h>
#include <dpl/m17/variant.h>
#include <gtest/gtest.h>
#include <string>

TEST(dpl_bbp_promise, basic) {
  dpl::bbp::promise<int> p([](auto fulfill, auto reject) { fulfill(3); });
}

TEST(dpl_bbp_promise, empty_promise) {
  dpl::bbp::promise<> p([](auto fulfill, auto reject) { fulfill(); });
}

// Check that when the then function has a 'void' return, then the
// corresponding promise is an empty promise.
TEST(dpl_bbp_promise, void_then) {
  dpl::bbp::promise<> foo = dpl::bbp::promise<>([](auto fulfill, auto reject) {
                         fulfill();
                       }).then([]() {});

  dpl::bbp::promise<> bar = dpl::bbp::promise<int>([](auto fulfill, auto reject) {
                         fulfill(3);
                       }).then([](int) {}, [](std::exception_ptr) {});
}

TEST(dpl_bbp_promise, then_two_arg) {
  //  Note the error message here. Concepts doesn't help.
  //  dpl::bbp::promise<int> q([](auto fulfill, auto reject) { fulfill("33"); });

  dpl::bbp::promise<std::string> ps =
      dpl::bbp::promise<int>([](auto fulfill, auto reject) { fulfill(3); })
          .then([](int i) { return std::string(std::to_string(i)); },
                [](std::exception_ptr) { return std::string("error"); });

  std::string result;
  ps.then(
      [&](auto s) {
        result = s;
        return dpl::m17::monostate();
      },
      [&](auto e) {
        result = "error";
        return dpl::m17::monostate();
      });
  EXPECT_EQ(result, "3") << "The then function wasn't called.";

  result = "";
  ps.then(
        [&](auto s) {
          throw std::runtime_error("error");
          return dpl::m17::monostate();
        },
        [&](auto e) {
          result = "error";
          return dpl::m17::monostate();
        })
      .then(
          [&](auto s) {
            result = "value";
            return dpl::m17::monostate();
          },
          [&](std::exception_ptr e) {
            result = "expected_error";
            try {
              std::rethrow_exception(e);
            } catch (const std::runtime_error &error) {
              EXPECT_EQ(error.what(), std::string("error"));
            } catch (...) {
              ADD_FAILURE() << "Unexpected exception thrown.";
            }
            return dpl::m17::monostate();
          });
  EXPECT_EQ(result, "expected_error") << "Error handling didn't happen.";
}

TEST(dpl_bbp_promise, then_one_arg) {
  std::string result;
  dpl::bbp::promise<int>([](auto fulfill, auto reject) {
    fulfill(3);
  }).then([](int i) {
      return std::string(std::to_string(i));
    }).then([&](auto s) {
    result = s;
    return dpl::m17::monostate();
  });
  EXPECT_EQ(result, "3") << "The then function wasn't called.";

  result = "";
  dpl::bbp::promise<dpl::m17::monostate>(
      [](auto fulfill, auto reject) { fulfill(dpl::m17::monostate()); })
      .then([](dpl::m17::monostate) -> dpl::m17::monostate {
        throw std::runtime_error("exception");
      })
      .then(
          [&](auto s) {
            result = "value";
            return dpl::m17::monostate();
          },
          [&](std::exception_ptr e) {
            result = "expected_error";
            try {
              std::rethrow_exception(e);
            } catch (const std::runtime_error &error) {
              EXPECT_EQ(error.what(), std::string("exception"));
            } catch (...) {
              ADD_FAILURE() << "Unexpected exception thrown.";
            }
            return dpl::m17::monostate();
          });
  EXPECT_EQ(result, "expected_error") << "Error handling didn't happen.";
}

TEST(dpl_bbp_promise, fulfill) {
  dpl::bbp::promise<int, double> p = dpl::bbp::promise<>::fulfill(3, 2.5);

  bool fulfilled = false;
  p.then([&fulfilled](int i, double d) {
    fulfilled = true;
    EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
    EXPECT_EQ(d, 2.5) << "Unexpected value in fulfilled promise.";
  });
  EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
}

TEST(dpl_bbp_promise, reject) {
  std::exception_ptr error;
  try {
    throw std::runtime_error("test");
  } catch (...) {
    error = std::current_exception();
  }

  dpl::bbp::promise<int, double> p = dpl::bbp::promise<>::reject<int, double>(error);

  bool rejected = false;
  p.then([](int i, double d) { ADD_FAILURE() << "Unexpected fulfillment."; },
         [&rejected, &error](std::exception_ptr e) {
           EXPECT_EQ(e, error) << "Rejected with wrong exception";
           rejected = true;
         });
  EXPECT_TRUE(rejected) << "Promise wasn't rejected.";
}

TEST(dpl_bbp_promise, then_promise_promise) {
  dpl::bbp::promise<> p = dpl::bbp::promise<>::fulfill();

  {
    dpl::bbp::promise<int> p2 = p.then([] { return dpl::bbp::promise<>::fulfill(3); });

    bool fulfilled = false;
    p2.then([&fulfilled](int i) {
      fulfilled = true;
      EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
  }

  {
    dpl::bbp::promise<int> p2 =
        p.then([] { return dpl::bbp::promise<>::fulfill(4); },
               [](std::exception_ptr e) { return dpl::bbp::promise<>::fulfill(2); });

    bool fulfilled = false;
    p2.then([&fulfilled](int i) {
      fulfilled = true;
      EXPECT_EQ(i, 4) << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
  }
}

TEST(dpl_bbp_promise, then_tuple) {
  dpl::bbp::promise<> p = dpl::bbp::promise<>::fulfill();

  {
    dpl::bbp::promise<int, std::string> p2 =
        p.then([] { return std::make_tuple(3, std::string("test")); });

    bool fulfilled = false;
    p2.then([&fulfilled](int i, const std::string & s) {
      fulfilled = true;
      EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
      EXPECT_EQ(s, "test") << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
  }
  {
    dpl::bbp::promise<int, std::string> p2 =
        p.then([] { return std::make_tuple(3, std::string("test")); },
               [](std::exception_ptr e) {
                ADD_FAILURE() << "Unexpected exception thrown.";
                return std::make_tuple(4, std::string("test2"));
               }
               );

    bool fulfilled = false;
    p2.then([&fulfilled](int i, const std::string & s) {
      fulfilled = true;
      EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
      EXPECT_EQ(s, "test") << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
  }
}

// TODO: add some tests that interact with asio.
//       - ASIO_STANDALONE is the definition required.
// TODO: add test that exceptions in the resolver are rethrown.
// TODO: add test that in the two-function version the exception handler is
// called.
// TODO: Test that promise<std::exception_ptr> works properly.

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
