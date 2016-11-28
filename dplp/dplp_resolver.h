#ifndef DPLP_RESOLVER_INCLUDED
#define DPLP_RESOLVER_INCLUDED

#include <dplmrts_invocable.h>
#include <dplmrts_invocablearchetype.h>

#include <exception> // std::exception_ptr

namespace dplp {
// Types that satisfy 'Resolver<Types...>' are callable with their first
// argument satisfying 'dplmrts::Invocable<Types...>' and their second argument
// satisfying 'dplmrts::Invocable<std::exception_ptr>'.
template <typename F, typename... Types>
concept bool Resolver =
    dplmrts::Invocable<F, dplmrts::InvocableArchetype<Types...>,
                       dplmrts::InvocableArchetype<std::exception_ptr>>;

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
