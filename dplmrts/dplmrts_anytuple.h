#ifndef DPLMRTS_ANYTUPLE_INCLUDED
#define DPLMRTS_ANYTUPLE_INCLUDED

#include <tuple>

namespace dplmrts {
template <typename T>
concept bool AnyTuple = requires(T t){{t}->std::tuple<auto...>};
}

#endif
