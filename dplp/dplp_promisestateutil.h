#ifndef INCLUDED_DPLP_PROMISESTATEUTIL
#define INCLUDED_DPLP_PROMISESTATEUTIL

#include <dplm17_variant.h> // dplm17::get
#include <dplp_promisestate.h>
#include <experimental/tuple> // std::experimental::apply

namespace dplp {

class PromiseStateUtil {
  // TODO: Move these elsewhere.
  static constexpr std::size_t waiting_state = 0; // Not fulfilled yet.

  static constexpr std::size_t fulfilled_state = 1; // Fulfilled with a
                                                    // sequence of values.

  static constexpr std::size_t rejected_state = 2; // Rejected with an
                                                   // exception.

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
  for (auto &&pf : dplm17::get<waiting_state>(promiseStateInWaiting->d_state))
    pf.first(fulfillValues...);

  // Move to the fulfilled state
  promiseStateInWaiting->d_state.template emplace<fulfilled_state>(
      std::move(fulfillValues)...);
}

template <typename... T>
void PromiseStateUtil::reject(
    dplp::PromiseState<T...> *const promiseStateInWaiting,
    std::exception_ptr error) {
  // Call all the waiting functions with the exception.
  for (auto &&pf : dplm17::get<waiting_state>(promiseStateInWaiting->d_state))
    pf.second(error);

  // Move to the rejected state.
  promiseStateInWaiting->d_state.template emplace<rejected_state>(
      std::move(error));
}

template <typename FulfilledCont, typename RejectedCont, typename... Types>
void PromiseStateUtil::postContinuations(
    dplp::PromiseState<Types...> *const promiseState,
    FulfilledCont fulfilledCont, RejectedCont rejectedCont) {
  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(promiseState->d_state)) {
    waitingFunctions->emplace_back(std::move(fulfilledCont),
                                   std::move(rejectedCont));
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(promiseState->d_state)) {
    std::experimental::apply(std::move(fulfilledCont), *fulfilledValues);
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(promiseState->d_state);
    std::invoke(std::move(rejectedCont), rejectedException);
  }
}
}

#endif
