#ifndef BPLBBP_ISPROMISE_INCLUDED
#define BPLBBP_ISPROMISE_INCLUDED

namespace dplbbp {
template <typename... Types> class promise;

template <typename T>
concept bool AnyPromise = requires(T t){{t}->promise<auto...>};
}

#endif
