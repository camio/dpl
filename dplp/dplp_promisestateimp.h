#ifndef INCLUDED_DPLP_PROMISESTATEIMP
#define INCLUDED_DPLP_PROMISESTATEIMP

//@PURPOSE: Provide datatypes for representing promise state.
//
//@CLASSES:
//  dplp::PromiseStateImp: general promise state
//  dplp::PromiseStateImpFulfilled: fulfilled promise datatype
//  dplp::PromiseStateImpRejected: rejected promise datatype
//  dplp::PromiseStateImpWaiting: waiting promise datatype
//
//@SEE_ALSO: dplp_promisestateimputil
//
//@DESCRIPTION: This component provides several low-level datatypes that
// represent the internal state for promise datatypes. The principle type is
// 'dplp::PromiseStateImp' which is a variant over the three promise states
// which are represented by 'dplp::PromiseStateImpFulfilled',
// 'dplp::PromiseStateImpRejected', and 'dplp::PromiseStateImpWaiting'.
//
// 'dplp::PromiseStateImp' also contains a 'std::mutex' which is intended to
// protect its other data member. Use of this mutex is not enforced in any way.
// The expectation is that higher-level components will insulate the user from
// incorect mutex usage.

#include <dplm17_variant.h>

#include <exception>   // std::exception_ptr
#include <functional>  // std::function
#include <mutex>       // std::mutex
#include <tuple>       // std::tuple
#include <utility>     // std::pair
#include <vector>

namespace dplp {

template <typename... Types>
struct PromiseStateImpWaiting {
    // This class is a value semantic type that implements the internal state
    // of a promise in the waiting state. The template parameters correspond to
    // the types of the values this promise contains.

    // The waiting state includes a list of functions to be called when
    // fulfilment or rejection occurs.
    std::vector<std::pair<std::function<void(Types...)>,
                          std::function<void(std::exception_ptr)> > >
        d_continuations;
};

template <typename... Types>
struct PromiseStateImpFulfilled {
    // This class is a value semantic type that implements the internal state
    // of a fulfilled promise. The template parameters correspond to the types
    // of values this promise contains.

    // The fulfilled state includes a tuple of the fulfilled values.
    std::tuple<Types...> d_values;
};

struct PromiseStateImpRejected {
    // This class is a value semantic type that implements the internal state
    // of a rejected promise.

    // The rejected state includes an 'exception_ptr'.
    std::exception_ptr d_error;
};

template <typename... Types>
struct PromiseStateImp {
    // This class is a value semantic type that implements the internal state
    // stored by a promise implementation.

    // Some of the three states carry data.
    dplm17::variant<PromiseStateImpWaiting<Types...>,
                    PromiseStateImpFulfilled<Types...>,
                    PromiseStateImpRejected>
               d_state;

    std::mutex d_mutex;
};
}

#endif
