#ifndef DPLP_ANYPROMISE_INCLUDED
#define DPLP_ANYPROMISE_INCLUDED

namespace dplp {
template <typename... Types> class promise;

template <typename T>
concept bool AnyPromise = requires(T t){{t}->promise<auto...>};
}

#endif
