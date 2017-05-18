#ifndef INCLUDED_DPLP_ANYPROMISE
#define INCLUDED_DPLP_ANYPROMISE

//@PURPOSE: Provide a concept that is satisfied by promise types.
//
//@CONCEPTS:
//  dplp::AnyPromise: concept satisified by promises
//
//@SEE ALSO: dplp_promise
//
//@DESCRIPTION: This component provides a concept that is satisfied by any
// 'dplp::Promise' type. This is intended to be used to help aid SFINE when the
// "promisness" of a type needs to be queried on the result of a metafunction.
//
// Note that this component depends "in name only" on dplp_promise.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Overload when the nested type is a promise
///- - - - - - - - - - - - - - - - - - - - - - - - - - -
// Suppose that we want to write a function that takes in a container and has a
// special overload when the container has promise elements. We presume the
// existence of a conforming `Container` concept from the Ranges TS.
//
// First, we declare the signature for the non-tuple overload:
//..
//  template<Container C>
//  requires !dplp::AnyPromise<typename C::value_type>
//  int f(C c);
//..
// Finally, we declare the signature for the tuple overload:
//..
//  template<Container C>
//  requires dplp::AnyPromise<typename C::value_type>
//  int f(C c);
//..

//TODO: verify whether or not this is correct
#include <type_traits> // std::true_type, std::false_type

namespace dplp {
template <typename... Types>
class Promise;

//template <typename T>
//concept bool AnyPromise =
//    // This concept is satisified by `dplp::Promise` template instantiations.
//    requires(T t){{t}->Promise<auto...>};

template <typename T>
struct IsPromise : std::false_type {
};

template <typename... T>
struct IsPromise<Promise<T...> > : std::true_type {
};

template <class T>
constexpr bool isPromise = IsPromise<T>::value;
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
