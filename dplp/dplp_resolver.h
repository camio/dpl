#ifndef INCLUDED_DPLP_RESOLVER
#define INCLUDED_DPLP_RESOLVER

//@PURPOSE: Provide a concept that is satisfied by promise resolver functions.
//
//@CONCEPTS:
//  dplp::Resolver: concept satisified by resolvers
//
//@SEE ALSO: dplp_promise
//
//@DESCRIPTION: This component provides a concept that is satisfied by promise
// resolvers. Promise resolvers are invocable objects that accept two
// arguments: the first being the 'resolve' function and the second being the
// 'reject' function.
//
// Each promise is associated with a list of types and so also is a promise
// resolver. The 'resolve' function takes in a value of each of these types as
// a parameter. The 'reject' function, on the other hand, always takes in a
// single 'std::exception_ptr' argument.
//
///Limitations in Negative Recognition
///-----------------------------------
// Sometimes 'dplp::Resolver' will result in a compilation error instead of
// returning a negative result for certain resolver function. Consider the
// following 'int' resolver.
//..
//  auto intResolver = [](auto fulfill, auto reject){ fulfill(3); };
//..
// If we were to see if this were a 'std::string' resolver using,
//..
//  dplp::Resolver<decltype(intResolver),std::string>
//..
// , we would get a compilation error instead of a negative result. The exact
// cause of this behavior is as of yet unknown. As a workaround, we suggest
// writing 'intResolver' as follows:
//..
//  auto intResolver = [](Invocable<int> fulfill, auto reject){ fulfill(3); };
//..
// Note that what we did is properly constrain the type of the lambda.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Create a promise factory
///- - - - - - - - - - - - - - - - - -
// Suppose we want to write a function that builds a 'dplp::Promise' object. We
// use 'dplp::Resolver' to ensure that a proper resolver is passed to the
// constructor.
//..
//  template<typename T>
//  dplp::Promise<T> makePromise(dplp::Resolver<T> r) {
//    return dplp::Promise<T>(r);
//  }
//..

#include <dplmrts_invocable.h>
#include <dplmrts_invocablearchetype.h>

#include <exception>  // std::exception_ptr

namespace dplp {
// Types that satisfy 'Resolver<Types...>' are callable with their first
// argument satisfying 'dplmrts::Invocable<Types...>' and their second argument
// satisfying 'dplmrts::Invocable<std::exception_ptr>'.
template <typename F, typename... Types>
concept bool Resolver =
    dplmrts::Invocable<F,
                       dplmrts::InvocableArchetype<Types...>,
                       dplmrts::InvocableArchetype<std::exception_ptr> >;

// What follows is an alternate definition of the same concept.
//
// template <typename F, typename... Types>
// concept bool Resolver = requires(F f, Types... t)
// {
//     f(dplmrts::InvocableArchetype<Types...>(),
//       dplmrts::InvocableArchetype<std::exception_ptr>());
// };
}

#endif
