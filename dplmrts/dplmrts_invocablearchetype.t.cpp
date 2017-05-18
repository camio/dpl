#include <dplmrts_invocablearchetype.h>

#include <gtest/gtest.h>

#include <functional>  // std::function
#include <string>

TEST(dplmrts_invocable, basic)
{
    EXPECT_EQ((dplmrts::Invocable<dplmrts::InvocableArchetype<int>, int>),
              true)
        << "Archetype doesn't match concept";
    EXPECT_EQ(
        (dplmrts::Invocable<dplmrts::InvocableArchetype<std::string>, int>),
        false)
        << "Archetype doesn't match concept";
    EXPECT_EQ((dplmrts::Invocable<int, int>), false)
        << "Archetype doesn't match concept";
    EXPECT_EQ(
        (dplmrts::Invocable<dplmrts::InvocableArchetype<int, std::string>,
                            int,
                            std::string>),
        true)
        << "Archetype doesn't match concept";
}

inline
int foo(auto value)
{
    return 0;
}

inline
int foo(dplmrts::Invocable<dplmrts::InvocableArchetype<int> > invocable)
{
    return 1;
}

TEST(dplmrts_invocable, example)
{
    EXPECT_EQ(foo(0), 0) << "The wrong overload was selected.";
    EXPECT_EQ(foo(std::function<void(std::function<void(int)>)>()), 1)
        << "The wrong overload was selected.";
    EXPECT_EQ(foo(std::function<void(std::function<void(std::string)>)>()), 0)
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
