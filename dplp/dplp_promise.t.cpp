#include <dplp_promise.h>

#include <dplm17_variant.h>
#include <gtest/gtest.h>

#include <string>

template <typename... Types>
struct promise2 {
    // Two-argument version of case #1
    template <
        typename FulfilledCont,
        typename RejectedCont,
        std::enable_if_t<
            // void return type
            std::is_void<std::result_of_t<FulfilledCont(Types...)> >::value &&
                // 'fulfilledCont' and 'rejectedCont' have matching
                // return values
                std::is_same<std::result_of_t<FulfilledCont(Types...)>,
                             std::result_of_t<
                                 RejectedCont(std::exception_ptr)> >::value,
            int> = 0>
    int myThen(FulfilledCont fulfilledCont, RejectedCont rejectedCont) const
    {
        return 0;
    }

    // One-argument version of case #1
    template <
        typename FulfilledCont,
        std::enable_if_t<
            // void return type
            std::is_void<std::result_of_t<FulfilledCont(Types...)> >::value,
            int> = 0>
    int myThen(
         FulfilledCont fulfilledCont) const  // One-argument version of case #1
    {
        return 1;
    }

    template <
        typename FulfilledCont,
        typename RejectedCont,
        std::enable_if_t<
            // tuple return type
            dplmrts::is_tuple_v<std::result_of_t<FulfilledCont(Types...)> > &&
                // 'fulfilledCont' and 'rejectedCont' have matching return
                // values
                std::experimental::is_same_v<
                    std::result_of_t<FulfilledCont(Types...)>,
                    std::result_of_t<RejectedCont(std::exception_ptr)> >,
            int> = 0>
    int
    myThen(FulfilledCont fulfilledCont,
           RejectedCont rejectedCont) const  // Two-argument version of case #2
    {
        return 2;
    }
};

TEST(dplp_promise, experiments)
{
    ASSERT_EQ(0,
              promise2<int>().myThen([](int i) {},
                                     [](const std::exception_ptr&) {}));
    ASSERT_EQ(1, promise2<int>().myThen([](int i) {}));

    ASSERT_EQ(
        2,
        promise2<int>().myThen(
            [](int i) { return std::make_tuple(1, 2); },
            [](const std::exception_ptr&) { return std::make_tuple(1, 2); }));
}

TEST(dplp_promise, basic)
{
    dplp::Promise<int> p([](auto fulfill, auto reject) { fulfill(3); });
}

TEST(dplp_promise, empty_promise)
{
    dplp::Promise<> p([](auto fulfill, auto reject) { fulfill(); });
}

// Check that when the then function has a 'void' return, then the
// corresponding promise is an empty promise.
TEST(dplp_promise, void_then)
{
    dplp::Promise<> foo = dplp::Promise<>([](auto fulfill, auto reject) {
                              fulfill();
                          }).then([]() {});

    dplp::Promise<> bar = dplp::Promise<int>([](auto fulfill, auto reject) {
                              fulfill(3);
                          }).then([](int) {}, [](std::exception_ptr) {});
}

TEST(dplp_promise, then_two_arg)
{
    //  Note the error message here. Concepts doesn't help.
    //  dplp::Promise<int> q([](auto fulfill, auto reject) { fulfill("33"); });

    dplp::Promise<std::string> ps =
        dplp::Promise<int>([](auto fulfill, auto reject) { fulfill(3); })
            .then([](int i) { return std::string(std::to_string(i)); },
                  [](std::exception_ptr) { return std::string("error"); });

    std::string result;
    ps.then(
        [&](auto s) {
            result = s;
            return dplm17::monostate();
        },
        [&](auto e) {
            result = "error";
            return dplm17::monostate();
        });
    EXPECT_EQ(result, "3") << "The then function wasn't called.";

    result = "";
    ps.then(
          [&](auto s) {
              throw std::runtime_error("error");
              return dplm17::monostate();
          },
          [&](auto e) {
              result = "error";
              return dplm17::monostate();
          })
        .then(
            [&](auto s) {
                result = "value";
                return dplm17::monostate();
            },
            [&](std::exception_ptr e) {
                result = "expected_error";
                try {
                    std::rethrow_exception(e);
                }
                catch (const std::runtime_error& error) {
                    EXPECT_EQ(error.what(), std::string("error"));
                }
                catch (...) {
                    ADD_FAILURE() << "Unexpected exception thrown.";
                }
                return dplm17::monostate();
            });
    EXPECT_EQ(result, "expected_error") << "Error handling didn't happen.";
}

TEST(dplp_promise, then_one_arg)
{
    std::string result;
    dplp::Promise<int>([](auto fulfill, auto reject) {
        fulfill(3);
    }).then([](int i) {
          return std::string(std::to_string(i));
      }).then([&](auto s) {
        result = s;
        return dplm17::monostate();
    });
    EXPECT_EQ(result, "3") << "The then function wasn't called.";

    result = "";
    dplp::Promise<dplm17::monostate>(
        [](auto fulfill, auto reject) { fulfill(dplm17::monostate()); })
        .then([](dplm17::monostate) -> dplm17::monostate {
            throw std::runtime_error("exception");
        })
        .then(
            [&](auto s) {
                result = "value";
                return dplm17::monostate();
            },
            [&](std::exception_ptr e) {
                result = "expected_error";
                try {
                    std::rethrow_exception(e);
                }
                catch (const std::runtime_error& error) {
                    EXPECT_EQ(error.what(), std::string("exception"));
                }
                catch (...) {
                    ADD_FAILURE() << "Unexpected exception thrown.";
                }
                return dplm17::monostate();
            });
    EXPECT_EQ(result, "expected_error") << "Error handling didn't happen.";
}

TEST(dplp_promise, fulfill)
{
    dplp::Promise<int, double> p = dplp::makeFulfilledPromise(3, 2.5);

    bool fulfilled = false;
    p.then([&fulfilled](int i, double d) {
        fulfilled = true;
        EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
        EXPECT_EQ(d, 2.5) << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
}

TEST(dplp_promise, reject)
{
    std::exception_ptr error =
        std::make_exception_ptr(std::runtime_error("test"));

    dplp::Promise<int, double> p =
        dplp::makeRejectedPromise<int, double>(error);

    bool rejected = false;
    p.then([](int i, double d) { ADD_FAILURE() << "Unexpected fulfillment."; },
           [&rejected, &error](std::exception_ptr e) {
               EXPECT_EQ(e, error) << "Rejected with wrong exception";
               rejected = true;
           });
    EXPECT_TRUE(rejected) << "Promise wasn't rejected.";
}

TEST(dplp_promise, then_promise_promise)
{
    dplp::Promise<> p = dplp::makeFulfilledPromise();

    {
        dplp::Promise<int> p2 =
            p.then([] { return dplp::makeFulfilledPromise(3); });

        bool fulfilled = false;
        p2.then([&fulfilled](int i) {
            fulfilled = true;
            EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
        });
        EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
    }

    {
        dplp::Promise<int> p2 =
            p.then([] { return dplp::makeFulfilledPromise(4); },
                   [](std::exception_ptr e) {
                       return dplp::makeFulfilledPromise(2);
                   });

        bool fulfilled = false;
        p2.then([&fulfilled](int i) {
            fulfilled = true;
            EXPECT_EQ(i, 4) << "Unexpected value in fulfilled promise.";
        });
        EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
    }
}

TEST(dplp_promise, then_tuple)
{
    dplp::Promise<> p = dplp::makeFulfilledPromise();

    {
        dplp::Promise<int, std::string> p2 =
            p.then([] { return std::make_tuple(3, std::string("test")); });

        bool fulfilled = false;
        p2.then([&fulfilled](int i, const std::string& s) {
            fulfilled = true;
            EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
            EXPECT_EQ(s, "test") << "Unexpected value in fulfilled promise.";
        });
        EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
    }
    {
        dplp::Promise<int, std::string> p2 =
            p.then([] { return std::make_tuple(3, std::string("test")); },
                   [](std::exception_ptr e) {
                       ADD_FAILURE() << "Unexpected exception thrown.";
                       return std::make_tuple(4, std::string("test2"));
                   });

        bool fulfilled = false;
        p2.then([&fulfilled](int i, const std::string& s) {
            fulfilled = true;
            EXPECT_EQ(i, 3) << "Unexpected value in fulfilled promise.";
            EXPECT_EQ(s, "test") << "Unexpected value in fulfilled promise.";
        });
        EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
    }
}

namespace {
struct C {
    int f() const { return 4; }
};
}

TEST(dplp_promise, invoke)
{
    // Check that 'std::invoke' is being called with continuation functions. We
    // do this by providing member functions which do not work with the normal
    // call syntax.

    dplp::Promise<C> p = dplp::makeFulfilledPromise(C());

    bool fulfilled = false;

    std::experimental::apply(std::move(&C::f), std::tuple<C>(C()));

    p.then(&C::f).then([&fulfilled](int i) {
        fulfilled = true;
        EXPECT_EQ(i, 4) << "Unexpected value in fulfilled promise.";
    });
    EXPECT_TRUE(fulfilled) << "Promise wasn't fulfilled.";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// ----------------------------------------------------------------------------
// Copyright 2017 Bloomberg Finance L.P.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------- END-OF-FILE ----------------------------------
