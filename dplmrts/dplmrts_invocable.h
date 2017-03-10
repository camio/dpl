#ifndef INCLUDED_DPLMRTS_INVOCABLE
#define INCLUDED_DPLMRTS_INVOCABLE

//@PURPOSE: Provide a concept that is satisfied by invocable types.
//
//@CONCEPTS:
//  dplmrts::Invocable: concept satisified by invocable types
//
//@SEE_ALSO: dplmrts_invocablearchetype
//
//@DESCRIPTION: This component provides a concept that is satisfied by any
// invocable type. This is intended to be used to help aid SFINE when the
// "invocableness" of a type needs to be queried on the result of a
// metafunction.
//
// A type is invocable if it can be invoked by `std::invoke`. The standard
// defines the following invocable types:
//
// 1. Classes with a call operator (`operator()`).
// 2. Function pointers
// 3. Member function pointers
//
// Note that in early versions of the Concepts TS, this concept was called
// "Callable". As of 2016-11-15, there is a plan to rename this concept to
// "Invocable" based on a discussion at the 2016 Issaquah WG21 meeting.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Overload when invocable
///- - - - - - - - - - - - - - - - -
// Suppose that we want to write a function `foo` that takes in either an int
// or a zero-argument invocable that returns an int. If `foo` has an `int`
// parameter, it will output it, otherwise it will output the result of
// invoking its argument.
//
// First, we define the int overload:
//..
//  inline void foo(int i) {
//    std::cout << i << std::endl;
//  }
//..
// Finally, we define the invocable overload:
//..
//  template<dplmrts::Invocable F>
//  requires std::experimental::is_same_v<std::result_of_t<F()>, int>
//  void foo(F f) {
//    std::cout << std::invoke(f) << std::endl;
//  }
//..
// Note that we're using `is_same_v` and `result_of_t` to verify that the
// result of `F` has type `int`.

#include <functional>  // std::invoke

namespace dplmrts {

template <typename F, typename... Types>
concept bool Invocable = requires(F f, Types... t)
{
    std::invoke(f, t...);
};
// This concept is satisified by types that meet the requirements of
// `std::invoke`.
}
#endif
