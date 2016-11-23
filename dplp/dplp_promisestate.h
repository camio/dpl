#ifndef INCLUDED_DPLP_PROMISESTATE
#define INCLUDED_DPLP_PROMISESTATE

#include <dplm17_variant.h>
#include <exception> // std::exception_ptr
#include <functional> // std::function
#include <tuple> // std::tuple
#include <utility> // std::pair
#include <vector>
#include <tuple>

namespace dplp {

template <typename... Types>
struct PromiseState {
  // This class is a value semantic type that implements the internal state
  // stored by a promise implementation.

  // Some of the three states carry data.
  dplm17::variant<
      // The waiting state includes a list of functions to be called when
      // fulfilment or rejection occurs.
      std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>,

      // The fulfilled state includes a tuple of the fulfilled values.
      std::tuple<Types...>,

      // The rejected state includes an 'exception_ptr'.
      std::exception_ptr>
      d_state;
};
}

#endif
