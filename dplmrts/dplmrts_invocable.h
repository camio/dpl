#ifndef DPLMRTS_INVOCABLE_INCLUDED
#define DPLMRTS_INVOCABLE_INCLUDED

#include <functional>

namespace dplmrts {

// This is an abridged implementation of the currently named 'Callable' concept
// in the Range's TS. Note that we are calling this "invocable" reflecting LEWG
// consensus at Issaquah 2016.
template <typename F, typename... Types>
concept bool Invocable = requires(F f, Types... t) {
  std::invoke(f, t...);
};

}
#endif
