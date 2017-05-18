#ifndef INCLUDED_DPLMRTS_INVOCABLEARCHETYPE
#define INCLUDED_DPLMRTS_INVOCABLEARCHETYPE

//@PURPOSE: Provide an archetype for the 'Invocable' concept.
//
//@SEE_ALSO: dplmrts_invocable
//
//@DESCRIPTION: This component provides a single class template,
// 'InvocableArchetype', that satisfies the 'Invocable' concept when it has the
// same template arguments. Archtypes useful when one needs to write a
// higher-order concept ie. a concept whose definition requires an arbitrary
// type which satisfies another concept.
//
// Note that 'dplmrts::InvocableArchetype' is only one kind of entity which
// satisfies 'Invocable'. In paritcular, member functions and function pointers
// are not checked when using 'dplmrts::InvocableArchtype' in concept
// definitions.
//
//@CLASSES:
//  dplmrts::InvocableArchetype: class that satisfies 'Invocable' concept
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Overload when a invocable of an invocable
///- - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose we want to write a function 'foo' that is specialized for invocables
// that accept a single 'int' argument arguments.
//
// First we define the arbitrary default case overload.
// ..
//   inline int foo(auto value);
// ..
// Finally, we define the invocable of invocable overload.
// ..
//   inline int foo(
//       dplmrts::Invocable<dplmrts::InvocableArchetype<int>> invocable);
// ..
// If we call 'foo' with an argument of type 'std::function<void
// (std::function<void (int)> )>', the second overload will be selected.

#include <dplmrts_invocable.h>

namespace dplmrts {
template <typename... Types>
struct InvocableArchetype {
    // 'InvocableArchetype<Types...>' is a type that satisifies
    // 'dplmrts::Invocable<Types...>'.

    void operator()(Types...) const {}
};
}

#endif

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
