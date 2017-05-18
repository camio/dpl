#ifndef INCLUDED_DPLMRTS_ANYTUPLE
#define INCLUDED_DPLMRTS_ANYTUPLE

//@PURPOSE: Provide a concept that is satisfied by tuple types.
//
//@CONCEPTS:
//  dplmrts::AnyTuple: concept satisified by tuples
//
//@SEE ALSO:
//
//@DESCRIPTION: This component provides a concept that is satisfied by any
// 'std::tuple' type. This is intended to be used to help aid SFINE when the
// "tupleness" of a type needs to be queried on the result of a metafunction.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Overload when the nested type is a tuple
///- - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we want to write a function that takes in a container and has a
// special overload when the container has tuple elements. We presume the
// existence of a conforming `Container` concept from the Ranges TS.
//
// First, we declare the signature for the non-tuple overload:
//..
//  template<Container C>
//  requires !dplmrts::AnyTuple<typename C::value_type>
//  int f(C c);
//..
// Finally, we declare the signature for the tuple overload:
//..
//  template<Container C>
//  requires dplmrts::AnyTuple<typename C::value_type>
//  int f(C c);
//..

#include <tuple>

namespace dplmrts {

//template <typename T>
//concept bool AnyTuple =
//    // This concept is satisified by `std::tuple` template instantiations.
//    requires(T t){{t}->std::tuple<auto...>};

//TODO: get the right include for 'std::false_type' and 'std::true_type'
template <typename T>
struct is_tuple : std::false_type {
};

template <typename... T>
struct is_tuple<std::tuple<T...> > : std::true_type {
};

template <class T>
constexpr bool is_tuple_v = is_tuple<T>::value;
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
