#ifndef INCLUDED_DPLP_PROMISESTATE
#define INCLUDED_DPLP_PROMISESTATE

//@PURPOSE: Provide a low-level promise with only fundamental operations.
//
//@CLASSES:
//  dplp::PromiseState: low-level promise class
//
//@DESCRIPTION: This component provides a single class, 'dplp::PromiseState',
// which represents an asynchronous value, but with very low-level operations.
// In particular, it provides three operations: transition to fulfilled state,
// transition to rejected state, and posting continuations. The intent is that
// this class is used as a building block for higher-level promise data types,
// which provide an interface that's more suitable for use in applications.
//
// 'dplp::PromiseState' has three states: waiting, fulfilled, and rejected.
// Once constructed ''dplp::PromiseState' is in the waiting state. It can be
// moved to the fulfilled or rejected state with the 'fulfill' and 'reject'
// functions.
//
// Continuations can be added to 'dplp::PromiseState' using the
// 'postContinuations' function. Upon transition to the fulfilled or rejected
// state, posted continuations are executed. If 'dplp::PromiseState' is already
// in the fulfilled state, the effect of adding a continuation is that the
// continuation functions are executed immediately.
//
// Thread Safety
// -------------
// This class is fully thread safe.

#include <dplp_promisestateimp.h>
#include <dplp_promisestateimputil.h>

#include <utility>  // std::forward, std::move

namespace dplp {

template <typename... Types>
class PromiseState {
    // This class implements a low-level promise with only fundamental
    // operations. A default-constructed 'PromiseState' object is in the
    // "waiting" state.

    PromiseStateImp<Types...> d_imp;

  public:
    void fulfill(Types&&... fulfillValues);
        // Move to the "fulfilled" state using the specified 'fulfillValues'.
        // If there are any posted fulfilled continuations, call them with
        // 'fulfillValues'.

    void reject(std::exception_ptr error);
        // Move to the "rejected" state using the specified 'error'.  If there
        // are any posted rejected continuations, call them with 'error'.

    template <typename FulfilledCont, typename RejectedCont>
    void postContinuations(FulfilledCont&& fulfilledCont,
                           RejectedCont&&  rejectedCont);
        // Post the specified 'fulfulledCont' and 'rejectedCont' continuations.
        // More specifically, if in the waiting state add them to the list of
        // posted continuation. If in the 'fufilled' state, call
        // 'fulfilledCont' with the fulfill values. Finally, if in the rejected
        // state, call 'rejectedCont' with the rejected value.
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... Types>
void PromiseState<Types...>::fulfill(Types&&... fulfillValues)
{
    dplp::PromiseStateImpUtil::fulfill(&d_imp,
                                       std::forward<Types>(fulfillValues)...);
}

template <typename... Types>
void PromiseState<Types...>::reject(std::exception_ptr error)
{
    dplp::PromiseStateImpUtil::reject(&d_imp, std::move(error));
}

template <typename... Types>
template <typename FulfilledCont, typename RejectedCont>
void PromiseState<Types...>::postContinuations(FulfilledCont&& fulfilledCont,
                                               RejectedCont&&  rejectedCont)
{
    dplp::PromiseStateImpUtil::postContinuations(
        &d_imp,
        std::forward<FulfilledCont>(fulfilledCont),
        std::forward<RejectedCont>(rejectedCont));
}
}
#endif
