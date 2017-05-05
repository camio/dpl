#ifndef INCLUDED_DPLP_PROMISESTATEIMPUTIL
#define INCLUDED_DPLP_PROMISESTATEIMPUTIL

//@PURPOSE: Provide utility functions for 'dplp::PromiseStateImp' objects.
//
//@CLASSES:
//  dplp::PromiseStateImpUtil: 'dplp::PromiseStateImp' utilities
//
//@SEE_ALSO: dplp_promisestateimp
//
//@DESCRIPTION: This component provides a utility class
// 'dplp::PromiseStateImpUtil' which includes several functions that transition
// 'dplp::PromiseStateImp' to different states in a safe way. That is, the
// mutex is used and the state transitions follow the normal promise rules
// (e.g. one cannot transsition from a fulfilled or rejected promised back to a
// waiting promise)

#include <dplm17_variant.h>  // dplm17::get, dplm17::visit
#include <dplm20_overload.h>
#include <dplp_promisestateimp.h>

#include <experimental/tuple>  // std::experimental::apply
#include <mutex>               // std::lock_guard, std::mutex

namespace dplp {

class PromiseStateImpUtil {
    // This is a utility class that implements the core promise state
    // operations: 'fulfill', 'reject', and 'postContinuations'. These
    // functions can be safely called in multiple threads as long as no other
    // threads are modifying the 'dplp::PromiseStateImp' outside of these
    // functions.
    //
    // The implementation takes advantage of the fact that a 'PromiseStateImp',
    // assuming only these functions are used, cannot move to the waiting state
    // if it is already in a fufilled or rejected state.

  public:
    template <typename... T, typename... V>
    static void
    fulfill(dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
            V&&...                             fulfillValues);
        // Move the specified 'promiseStateInWaiting' to the fulfilled state
        // with the specified 'fulfillValues'. Call the 'first' of all the
        // waiting functions with 'fulfillValues'. The 'promiseStateInWaiting'
        // is moved to the fulfilled state before the waiting functions are
        // called. The behavior is undefined unless the specified
        // 'promiseStateInWaiting' is in the waiting state.

    template <typename... T>
    static void
    reject(dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
           std::exception_ptr                 error);
        // Move the specified 'promiseStateInWaiting' to the rejected state
        // with the specified 'error'. Call the 'second' of all the waiting
        // functions with 'error'. The 'promiseStateInWaiting' is moved to the
        // fulfilled state before the waiting functions are called. The
        // behavior is undefined unless the specified 'promiseStateInWaiting'
        // is in the waiting state.

    template <typename FulfilledCont, typename RejectedCont, typename... Types>
    static void
    postContinuations(dplp::PromiseStateImp<Types...> *const promiseState,
                      FulfilledCont&&                        fulfilledCont,
                      RejectedCont&&                         rejectedCont);
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... T, typename... V>
void PromiseStateImpUtil::fulfill(
                      dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
                      V&&...                             fulfillValues)
{
    std::vector<std::pair<std::function<void(T...)>,
                          std::function<void(std::exception_ptr)> > >
        continuations;
    {
        const std::lock_guard<std::mutex> lock(promiseStateInWaiting->d_mutex);

        // Note that we need to delay calling the continuation functions in
        // case they attempt to add more continuations.
        continuations = std::move(dplm17::get<PromiseStateImpWaiting<T...> >(
                                      promiseStateInWaiting->d_state)
                                      .d_continuations);
        // Move to the fulfilled state
        promiseStateInWaiting->d_state = PromiseStateImpFulfilled<T...>{
            {std::forward<V>(fulfillValues)...}};

        // Note that due to the 'std::forward', we cannot use 'fulfillValues'
        // after this point.
    }

    // Call all the waiting functions with the fulfill values.
    const auto& values = dplm17::get<PromiseStateImpFulfilled<T...> >(
                                                promiseStateInWaiting->d_state)
                             .d_values;
    for (auto&& pf : continuations)
        std::experimental::apply(pf.first, values);
}

template <typename... T>
void PromiseStateImpUtil::reject(
                      dplp::PromiseStateImp<T...> *const promiseStateInWaiting,
                      std::exception_ptr                 error)
{
    std::vector<std::pair<std::function<void(T...)>,
                          std::function<void(std::exception_ptr)> > >
        continuations;
    {
        const std::lock_guard<std::mutex> lock(promiseStateInWaiting->d_mutex);

        // Note that we need to delay calling the continuation functions in
        // case
        // they attempt to add more continuations.
        continuations = std::move(dplm17::get<PromiseStateImpWaiting<T...> >(
                                      promiseStateInWaiting->d_state)
                                      .d_continuations);
        // Move to the rejected state
        promiseStateInWaiting->d_state =
            PromiseStateImpRejected{std::move(error)};
    }

    // Call all the waiting functions with the fulfill values.
    const auto& errorValue =
        dplm17::get<PromiseStateImpRejected>(promiseStateInWaiting->d_state)
            .d_error;
    for (auto&& pf : continuations)
        pf.second(errorValue);
}

template <typename FulfilledCont, typename RejectedCont, typename... Types>
void PromiseStateImpUtil::postContinuations(
                          dplp::PromiseStateImp<Types...> *const promiseState,
                          FulfilledCont&&                        fulfilledCont,
                          RejectedCont&&                         rejectedCont)
{
    std::unique_lock<std::mutex> lock(promiseState->d_mutex);
    return dplm17::visit(
        dplm20::overload(
            [&](PromiseStateImpWaiting<Types...>& waitingState) {
                waitingState.d_continuations.emplace_back(
                    std::forward<FulfilledCont>(fulfilledCont),
                    std::forward<RejectedCont>(rejectedCont));
            },
            [&](const PromiseStateImpFulfilled<Types...>& fulfilledState) {
                // Note that we need to unlock the mutex in case
                // 'fulfilledCont'
                // results in another call that modifies 'promiseState'.
                lock.unlock();
                std::experimental::apply(
                    std::forward<FulfilledCont>(fulfilledCont),
                    fulfilledState.d_values);
            },
            [&](const PromiseStateImpRejected& rejectedState) {
                // Note that we need to unlock the mutex in case 'rejectedCont'
                // results in another call that modifies 'promiseState'.
                lock.unlock();
                std::invoke(std::forward<RejectedCont>(rejectedCont),
                            rejectedState.d_error);
            }),
        promiseState->d_state);
}
}

#endif
