#include <dplp_anypromise.h>

#include <dplp_promise.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(dplp_anypromise, basic)
{
    EXPECT_EQ((dplp::AnyPromise<dplp::Promise<int> >), true)
        << "promise<int> not detected as a promise";
    EXPECT_EQ((dplp::AnyPromise<dplp::Promise<int, std::string> >), true)
        << "promise<int,string> not detected as a promise";
    EXPECT_EQ((dplp::AnyPromise<int>), false) << "int detected as a promise";
}

namespace {
template <typename T>
concept bool Container = true;

template <Container                                    C>
requires !dplp::AnyPromise<typename C::value_type> int f(C c)
{
    return 0;
}

template <Container C>
requires dplp::AnyPromise<typename C::value_type> int f(C c)
{
    return 1;
}
}

TEST(dplp_anypromise, example)
{
    EXPECT_EQ(f(std::vector<int>()), 0) << "The wrong overload was selected.";
    EXPECT_EQ(f(std::vector<dplp::Promise<int> >()), 1)
        << "The wrong overload was selected.";
    EXPECT_EQ(f(std::vector<dplp::Promise<int, char> >()), 1)
        << "The wrong overload was selected.";
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
