#ifndef INCLUDED_DPLP_PROMISESTATE
#define INCLUDED_DPLP_PROMISESTATE

#include <dplp_promisestateimp.h>
#include <dplp_promisestateimputil.h>
#include <utility> // std::move

namespace dplp {

template <typename... Types> class PromiseState {
  PromiseStateImp<Types...> d_imp;

public:
  // TODO: make this a forwarding argument
  void fulfill(Types... fulfillValues);

  void reject(std::exception_ptr error);

  // TODO: make this a forwarding argument
  template <typename FulfilledCont, typename RejectedCont>
  void postContinuations(FulfilledCont fulfilledCont,
                         RejectedCont rejectedCont);
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... Types>
void PromiseState<Types...>::fulfill(Types... fulfillValues) {
  dplp::PromiseStateImpUtil::fulfill(&d_imp, fulfillValues...);
}

template <typename... Types>
void PromiseState<Types...>::reject(std::exception_ptr error) {
  dplp::PromiseStateImpUtil::reject(&d_imp, std::move(error));
}

template <typename... Types>
template <typename FulfilledCont, typename RejectedCont>
void PromiseState<Types...>::postContinuations(FulfilledCont fulfilledCont,
                                               RejectedCont rejectedCont) {
  dplp::PromiseStateImpUtil::postContinuations(&d_imp, fulfilledCont,
                                               rejectedCont);
}
}
#endif
