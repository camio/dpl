#ifndef DPLMRTS_ANYTUPLE_INCLUDED
#define DPLMRTS_ANYTUPLE_INCLUDED

//@PURPOSE: Provide a concept that is satisfied by tuple types.
//
//@CONCEPTS:
//  dplmrts::AnyTuple: concept satisified by tuples
//
//@SEE ALSO:
//
//@AUTHOR: David Sankel (dsankel@bloomberg.net)
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

template <typename T>
concept bool AnyTuple = requires(T t){{t}->std::tuple<auto...>};
// This concept is satisified by `std::tuple` template instantiations.

}

#endif
