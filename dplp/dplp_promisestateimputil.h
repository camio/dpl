#ifndef INCLUDED_DPLP_PROMISESTATEIMPUTIL
#define INCLUDED_DPLP_PROMISESTATEIMPUTIL

#include <dplm17_variant.h> // dplm17::get, dplm17::visit
#include <dplm20_overload.h>
#include <dplp_promisestateimp.h>

#include <experimental/tuple> // std::experimental::apply

namespace dplp {

class PromiseStateImpUtil {
  // This is a utility class that implements the core promise state operations:
  // 'fulfill', 'reject', and 'postContinuations'.

public:
  // Move the specified 'promiseStateInWaiting' to the fulfilled state with the
  // specified 'fulfillValues'. Call the 'first' of all the waiting functions
  // with 'fulfillValues'. The 'promiseStateInWaiting' is moved to the fulfilled
  // state before the waiting functions are called. The behavior is undefined
  // unless the specified 'promiseStateInWaiting' is in the waiting state.
  template <typename... T>
  static void fulfill(dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
                      T... fulfillValues);

  // Move the specified 'promiseStateInWaiting' to the rejected state with the
  // specified 'error'. Call the 'second' of all the waiting functions with
  // 'error'. The 'promiseStateInWaiting' is moved to the fulfilled state
  // before the waiting functions are called. The behavior is undefined unless
  // the specified 'promiseStateInWaiting' is in the waiting state.
  template <typename... T>
  static void reject(dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
                     std::exception_ptr error);

  template <typename FulfilledCont, typename RejectedCont, typename... Types>
  static void
  postContinuations(dplp::PromiseStateImp<Types...> *const promiseState,
                    FulfilledCont fulfilledCont, RejectedCont rejectedCont);
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... T>
void PromiseStateImpUtil::fulfill(
    dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
    T... fulfillValues) {
  // Call all the waiting functions with the fulfill values.
  for (auto &&pf :
       dplm17::get<PromiseStateImpWaiting<T...>>(promiseStateInWaiting->d_state)
           .d_continuations)
    pf.first(fulfillValues...);

  // Move to the fulfilled state
  promiseStateInWaiting->d_state =
      PromiseStateImpFulfilled<T...>{{std::move(fulfillValues)...}};
}

template <typename... T>
void PromiseStateImpUtil::reject(
    dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
    std::exception_ptr error) {
  // Call all the waiting functions with the exception.
  for (auto &&pf :
       dplm17::get<PromiseStateImpWaiting<T...>>(promiseStateInWaiting->d_state)
           .d_continuations)
    pf.second(error);

  // Move to the rejected state.
  promiseStateInWaiting->d_state = PromiseStateImpRejected{std::move(error)};
}

template <typename FulfilledCont, typename RejectedCont, typename... Types>
void PromiseStateImpUtil::postContinuations(
    dplp::PromiseStateImp<Types...> *const promiseState,
    FulfilledCont fulfilledCont, RejectedCont rejectedCont) {
  dplm17::visit(
      dplm20::overload(
          [&](PromiseStateImpWaiting<Types...> &waitingState) {
            waitingState.d_continuations.emplace_back(std::move(fulfilledCont),
                                                      std::move(rejectedCont));
          },
          [&](const PromiseStateImpFulfilled<Types...> &fulfilledState) {
            std::experimental::apply(std::move(fulfilledCont),
                                     fulfilledState.d_values);
          },
          [&](const PromiseStateImpRejected &rejectedState) {
            std::invoke(std::move(rejectedCont), rejectedState.d_error);
          }),
      promiseState->d_state);
}
}

#endif
