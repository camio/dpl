#include <dplmrts_invocable.h>

#include <gtest/gtest.h>

#include <experimental/type_traits>  // std::experimental::is_same_v
#include <functional>                // std::invoke
#include <type_traits>               // std::result_of_t
#include <vector>

namespace {

inline
int foo(int i)
{
    return 0;
}

template <dplmrts::Invocable F>
requires std::experimental::is_same_v<std::result_of_t<F()>, int> int foo(F f)
{
    return std::invoke(f);
}

int intGen()
{
    return 3;
}
}

TEST(dplmrts_invocable, example)
{
    EXPECT_EQ(foo(0), 0) << "The wrong overload was selected.";
    EXPECT_EQ(foo(intGen), 3) << "The wrong overload was selected.";
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
