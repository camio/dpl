#ifndef INCLUDED_DPLP_PROMISESTATEUTIL
#define INCLUDED_DPLP_PROMISESTATEUTIL

#include <dplm17_variant.h> // dplm17::get, dplm17::visit
#include <dplm20_overload.h>
#include <dplp_promisestate.h>
#include <experimental/tuple> // std::experimental::apply

namespace dplp {

class PromiseStateUtil {
  // This is a utility class that implements the core promise state operations:
  // 'fulfill', 'reject', and 'postContinuations'.

  // Move the specified 'promiseStateInWaiting' to the fulfilled state with the
  // specified 'fulfillValues'. Call the 'first' of all the waiting functions
  // with 'fulfillValues'. The 'promiseStateInWaiting' is moved to the fulfilled
  // state before the waiting functions are called. The behavior is undefined
  // unless the specified 'promiseStateInWaiting' is in the waiting state.
  template <typename... T>
  static void fulfill(dplp::PromiseState<T...> *const promiseStateInWaiting,
                      T... fulfillValues);

  // Move the specified 'promiseStateInWaiting' to the rejected state with the
  // specified 'error'. Call the 'second' of all the waiting functions with
  // 'error'. The 'promiseStateInWaiting' is moved to the fulfilled state
  // before the waiting functions are called. The behavior is undefined unless
  // the specified 'promiseStateInWaiting' is in the waiting state.
  template <typename... T>
  static void reject(dplp::PromiseState<T...> *const promiseStateInWaiting,
                     std::exception_ptr error);

  template <typename FulfilledCont, typename RejectedCont, typename... Types>
  static void
  postContinuations(dplp::PromiseState<Types...> *const promiseState,
                    FulfilledCont fulfilledCont, RejectedCont rejectedCont);
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... T>
void PromiseStateUtil::fulfill(
    dplp::PromiseState<T...> *const promiseStateInWaiting, T... fulfillValues) {
  // Call all the waiting functions with the fulfill values.
  for (auto &&pf :
       dplm17::get<PromiseStateWaiting<T...>>(promiseStateInWaiting->d_state)
           .d_continuations)
    pf.first(fulfillValues...);

  // Move to the fulfilled state
  promiseStateInWaiting->d_state =
      PromiseStateFulfilled<T...>{{std::move(fulfillValues)...}};
}

template <typename... T>
void PromiseStateUtil::reject(
    dplp::PromiseState<T...> *const promiseStateInWaiting,
    std::exception_ptr error) {
  // Call all the waiting functions with the exception.
  for (auto &&pf :
       dplm17::get<PromiseStateWaiting<T...>>(promiseStateInWaiting->d_state)
           .d_continuations)
    pf.second(error);

  // Move to the rejected state.
  promiseStateInWaiting->d_state = PromiseStateRejected{std::move(error)};
}

template <typename FulfilledCont, typename RejectedCont, typename... Types>
void PromiseStateUtil::postContinuations(
    dplp::PromiseState<Types...> *const promiseState,
    FulfilledCont fulfilledCont, RejectedCont rejectedCont) {
  dplm17::visit(dplm20::overload(
                    [&](PromiseStateWaiting<Types...> &waitingState) {
                      waitingState.d_continuations.emplace_back(
                          std::move(fulfilledCont), std::move(rejectedCont));
                    },
                    [&](const PromiseStateFulfilled<Types...> &fulfilledState) {
                      std::experimental::apply(std::move(fulfilledCont),
                                               fulfilledState.d_values);
                    },
                    [&](const PromiseStateRejected &rejectedState) {
                      std::invoke(std::move(rejectedCont),
                                  rejectedState.d_error);
                    }),
                promiseState->d_state);
}
}

#endif
