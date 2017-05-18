#include <dplp_resolver.h>

#include <dplp_promise.h>
#include <gtest/gtest.h>

#include <string>

TEST(dplp_resolver, basic)
{
    auto resolver = [](dplmrts::Invocable<int>                fulfill,
                       dplmrts::Invocable<std::exception_ptr> reject) {
        fulfill(3);
    };
    EXPECT_EQ((dplp::Resolver<decltype(resolver), int>), true)
        << "incorrectly not detected as a resolver";
    EXPECT_EQ((dplp::Resolver<decltype(resolver), std::string>), false)
        << "incorrectly detected a resolver";
    EXPECT_EQ((dplp::Resolver<int, int>), false)
        << "int detected as a resolver";
}

namespace {
template <typename T>
dplp::Promise<T> makePromise(dplp::Resolver<T> r)
{
    return dplp::Promise<T>(r);
}
}

TEST(dplp_resolver, example)
{
    dplp::Promise<int> r = makePromise<int>(
                             [](dplmrts::Invocable<int>                fulfill,
               dplmrts::Invocable<std::exception_ptr> reject) { fulfill(3); });
    bool fulfilled = false;
    r.then([&fulfilled](int i) {
        EXPECT_EQ(i, 3);
        fulfilled = true;
    });
    EXPECT_EQ(fulfilled, true);
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
