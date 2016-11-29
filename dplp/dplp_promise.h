#ifndef DPLP_PROMISE_INCLUDED
#define DPLP_PROMISE_INCLUDED

#include <dplmrts_anytuple.h>
#include <dplmrts_invocable.h>
#include <dplp_anypromise.h>
#include <dplp_promisestate.h>
#include <dplp_resolver.h>

#include <experimental/tuple> // std::experimental::apply
#include <experimental/type_traits> // std::experimental::is_void_v, std::experimental::is_same_v

#include <exception>   // std::exception_ptr
#include <functional>  // std::invoke
#include <tuple>       // std::tuple
#include <type_traits> // std::result_of_t

namespace dplp {

// 'Promise_TupleContinuationThenResult' is a type function that, when given a
// tuple type, returns a promise type that has fufillment types that match the
// element types of the tuple.
template <typename T> struct Promise_TupleContinuationThenResultImp {};
template <typename... T>
struct Promise_TupleContinuationThenResultImp<std::tuple<T...>> {
  using type = dplp::Promise<T...>;
};
template <dplmrts::AnyTuple T>
using Promise_TupleContinuationThenResult =
    typename Promise_TupleContinuationThenResultImp<T>::type;

// This class implements a value semantic type representing a heterogenius
// sequence of values or exception at some point in time.
//
// Note that this is a lot like a 'future', but with different constructors,
// simplified 'then' operations, and support for multiple fulfilled
// values.
template <typename... Types> class Promise {
  // Promises may be copied. There are no semantic problems with this since
  // they have no mutating members. Well, at least until a 'cancel' operation
  // is implemented anyway.
  std::shared_ptr<dplp::PromiseState<Types...>> d_data_sp;

public:
  // Create a new 'Promise' object based on the specified 'resolver'.
  // 'resolver' is called exactly once by this constructor with a
  // 'dplmrts::Invocable<Types...>' (resolve function) as its first and a
  // 'dplmrts::Invocable<std::exception_ptr>' (reject function) as its second
  // argument.  When the resolve function is called, this promise is fulfilled
  // with its arguments. When the reject function is called, this promise is
  // rejected with its argument. The behavior is undefined unless at most one
  // of the arguments to 'resolver' is ever called.
  //
  // Note that neither the reject nor resolve functions need be called from
  // within 'resolver'. 'resolver' could, for example, store these functions
  // elsewhere to be called at a later time.
  Promise(dplp::Resolver<Types...> resolver);

  // Return a new promise that, upon the fulfilment of this promise, will be
  // fulfilled with the result of the specified 'fulfilledCont' function or,
  // upon reject of this promise, will be rejected with the same
  // 'std::exception_ptr'.
  //
  // Optionally specify a 'rejectedCont' function, which provides the newly
  // created promise with a fulfillment value in case this promise is
  // rejected. 'fulfilledCont' called with arguments of type 'Types...' must
  // have the same return type as 'rejectedCont' when called with a
  // 'std::exception_ptr'.
  //
  // If either 'fulfilledCont' or 'rejectedCont' throw an exception, the
  // resulting promise will be a rejected promise containing that exception.
  //
  // The return type of 'fulfilledCont' controls the type of promise returned
  // from 'then'. There are the following cases:
  //
  // 1. If the return type of 'fulfilledCont' is 'void', the result of this
  //    function will be of type 'Promise<>'.
  // 2. If the return type of 'fulfilledCont' is 'tuple<T1,T2,...>' for some
  //    types 'T1,T2,...', the result of this function will be of type
  //    'Promise<T1,T2,...>'.
  // 3. If the return type of 'fulfilledCont' is 'Promise<T1,T2,...>' for
  //    some types 'T1,T2,...', the result of this function will be of type
  //    'Promise<T1,T2,...>'.
  // 4. If none of the above three conditions apply and the return type of
  //    'fulfilledCont' is 'T', then the result of this function will be of
  //    type 'Promise<T>'.

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
  Promise<> then(FulfilledCont fulfilledCont,
                 RejectedCont rejectedCont) // Two-argument version of case #1
      requires
      // void return type
      std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
  Promise<> then(FulfilledCont fulfilledCont) // One-argument version of case #1
      requires
      // void return type
      std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
  Promise_TupleContinuationThenResult<std::result_of_t<FulfilledCont(Types...)>>
  then(FulfilledCont fulfilledCont,
       RejectedCont rejectedCont) // Two-argument version of case #2
      requires
      // tuple return type
      dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
  Promise_TupleContinuationThenResult<std::result_of_t<FulfilledCont(Types...)>>
  then(FulfilledCont fulfilledCont) // One-argument version of case #2
      requires
      // tuple return type
      dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
  std::result_of_t<FulfilledCont(Types...)>
  then(FulfilledCont fulfilledCont,
       RejectedCont rejectedCont) // Two-argument version of case #3
      requires
      // promise return type
      dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
  std::result_of_t<FulfilledCont(Types...)>
  then(FulfilledCont fulfilledCont) // One-argument version of case #3
      requires
      // promise return type
      dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
      Promise<std::result_of_t<FulfilledCont(Types...)>>
      then(FulfilledCont fulfilledCont,
           RejectedCont rejectedCont) // Two-argument version of case #4
      requires
      // non-void return type
      !std::experimental::is_void_v<
          std::result_of_t<FulfilledCont(Types...)>> &&

      // non-tuple return type
      !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

      // non-promise return type
      !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
      Promise<std::result_of_t<FulfilledCont(Types...)>>
      then(FulfilledCont fulfilledCont) // One-argument version of case #4
      requires
      // non-void return type
      !std::experimental::is_void_v<
          std::result_of_t<FulfilledCont(Types...)>> &&

      // non-tuple return type
      !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

      // non-promise return type
      !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>>;

  // Return a promise with the specified types that is fulfilled with the
  // specified 'values'.
  template <typename... Types2>
  static Promise<Types2...> fulfill(Types2 &&... values);

  // Return a promise with the specified types that is rejected with the
  // specified 'error'.
  template <typename... Types2>
  static Promise<Types2...> reject(std::exception_ptr error);

private:
  // Create a new 'promise' object in the waiting state. It is never
  // fulfilled.
  Promise();
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... Types>
Promise<Types...>::Promise(dplp::Resolver<Types...> resolver)
    : d_data_sp(std::make_shared<PromiseState<Types...>>()) {
  // Set 'fulfil' to the fulfilment function. Note that it, as well as
  // reject, keeps its own shared pointer to 'd_data_sp'.
  auto fulfil = [d_data_sp = this->d_data_sp](Types... fulfillValues) noexcept {
    d_data_sp->fulfill(std::move(fulfillValues)...);
  };

  auto reject = [d_data_sp = this->d_data_sp](std::exception_ptr e) noexcept {
    d_data_sp->reject(std::move(e));
  };

  std::invoke(resolver, std::move(fulfil), std::move(reject));
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
Promise<> Promise<Types...>::then(
    FulfilledCont fulfilledCont,
    RejectedCont rejectedCont) // Two-argument version of case #1
    requires
    // void return type
    std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {

  return Promise<>([
    this, fulfilledCont = std::move(fulfilledCont),
    rejectedCont = std::move(rejectedCont)
  ](auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            std::invoke(std::move(fulfilledCont), t...);
            fulfill();
          } catch (...) {
            reject(std::current_exception());
          }
        },
        [ rejectedCont = std::move(rejectedCont), fulfill,
          reject ](std::exception_ptr e) mutable {
          try {
            std::invoke(std::move(rejectedCont), e);
            fulfill();
          } catch (...) {
            reject(std::current_exception());
          }
        });
  });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
Promise<> Promise<Types...>::then(
    FulfilledCont fulfilledCont) // One-argument version of case #1
    requires
    // void return type
    std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> {

  return Promise<>([ this, fulfilledCont = std::move(fulfilledCont) ](
      auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            std::invoke(std::move(fulfilledCont), t...);
            fulfill();
          } catch (...) {
            reject(std::current_exception());
          }
        },
        reject);
  });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
Promise_TupleContinuationThenResult<std::result_of_t<FulfilledCont(Types...)>>
Promise<Types...>::then(
    FulfilledCont fulfilledCont,
    RejectedCont rejectedCont) // Two-argument version of case #2
    requires
    // tuple return type
    dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  using Result = Promise_TupleContinuationThenResult<
      std::result_of_t<FulfilledCont(Types...)>>;

  return Result([
    this, fulfilledCont = std::move(fulfilledCont),
    rejectedCont = std::move(rejectedCont)
  ](auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            std::experimental::apply(fulfill, std::move(fulfilledCont)(t...));
          } catch (...) {
            reject(std::current_exception());
          }
        },
        [ rejectedCont = std::move(rejectedCont), fulfill,
          reject ](std::exception_ptr e) mutable {
          try {
            std::experimental::apply(fulfill,
                                     std::invoke(std::move(rejectedCont), e));
          } catch (...) {
            reject(std::current_exception());
          }
        });
  });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
Promise_TupleContinuationThenResult<std::result_of_t<FulfilledCont(Types...)>>
Promise<Types...>::then(
    FulfilledCont fulfilledCont) // One-argument version of case #2
    requires
    // tuple return type
    dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> {
  using Result = Promise_TupleContinuationThenResult<
      std::result_of_t<FulfilledCont(Types...)>>;

  return Result([ this, fulfilledCont = std::move(fulfilledCont) ](
      auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            std::experimental::apply(fulfill, std::move(fulfilledCont)(t...));
          } catch (...) {
            reject(std::current_exception());
          }
        },
        reject);
  });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
std::result_of_t<FulfilledCont(Types...)> Promise<Types...>::then(
    FulfilledCont fulfilledCont,
    RejectedCont rejectedCont) // Two-argument version of case #3
    requires
    // promise return type
    dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  using Result = std::result_of_t<FulfilledCont(Types...)>;

  return Result([
    this, fulfilledCont = std::move(fulfilledCont),
    rejectedCont = std::move(rejectedCont)
  ](auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            Result innerPromise = std::invoke(std::move(fulfilledCont), t...);
            innerPromise.d_data_sp->postContinuations(fulfill, reject);
          } catch (...) {
            reject(std::current_exception());
          }
        },
        [ rejectedCont = std::move(rejectedCont), fulfill,
          reject ](std::exception_ptr e) mutable {
          try {
            Result innerPromise = std::invoke(std::move(rejectedCont), e);
            innerPromise.d_data_sp->postContinuations(fulfill, reject);
          } catch (...) {
            reject(std::current_exception());
          }
        });
  });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
std::result_of_t<FulfilledCont(Types...)> Promise<Types...>::then(
    FulfilledCont fulfilledCont) // One-argument version of case #3
    requires
    // promise return type
    dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> {
  using Result = std::result_of_t<FulfilledCont(Types...)>;

  return Result([ this, fulfilledCont = std::move(fulfilledCont) ](
      auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            Result innerPromise = std::invoke(std::move(fulfilledCont), t...);
            innerPromise.d_data_sp->postContinuations(fulfill, reject);
          } catch (...) {
            reject(std::current_exception());
          }
        },
        reject);
  });
}

template <typename... Types>
    template <dplmrts::Invocable<Types...> FulfilledCont,
              dplmrts::Invocable<std::exception_ptr> RejectedCont>
    Promise<std::result_of_t<FulfilledCont(Types...)>> Promise<Types...>::then(
        FulfilledCont fulfilledCont,
        RejectedCont rejectedCont) // Two-argument version of case #4
    requires
    // non-void return type
    !std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> &&

    // non-tuple return type
    !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

    // non-promise return type
    !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  using U = std::result_of_t<FulfilledCont(Types...)>;

  return Promise<U>([
    this, fulfilledCont = std::move(fulfilledCont),
    rejectedCont = std::move(rejectedCont)
  ](auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            fulfill(std::invoke(std::move(fulfilledCont), t...));
          } catch (...) {
            reject(std::current_exception());
          }
        },
        [ rejectedCont = std::move(rejectedCont), fulfill,
          reject ](std::exception_ptr e) mutable {
          try {
            fulfill(std::invoke(std::move(rejectedCont), e));
          } catch (...) {
            reject(std::current_exception());
          }
        });
  });
}

template <typename... Types>
    template <dplmrts::Invocable<Types...> FulfilledCont>
    Promise<std::result_of_t<FulfilledCont(Types...)>> Promise<Types...>::then(
        FulfilledCont fulfilledCont) // One-argument version of case #4
    requires
    // non-void return type
    !std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> &&

    // non-tuple return type
    !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

    // non-promise return type
    !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> {
  using U = std::result_of_t<FulfilledCont(Types...)>;

  return Promise<U>([ this, fulfilledCont = std::move(fulfilledCont) ](
      auto fulfill, auto reject) mutable {
    d_data_sp->postContinuations(
        [ fulfilledCont = std::move(fulfilledCont), fulfill,
          reject ](Types... t) mutable {
          try {
            fulfill(std::invoke(std::move(fulfilledCont), t...));
          } catch (...) {
            reject(std::current_exception());
          }
        },
        [reject](std::exception_ptr e) mutable { reject(e); });
  });
}

template <typename... Types>
template <typename... Types2>
Promise<Types2...> Promise<Types...>::fulfill(Types2 &&... values) {
  Promise<Types2...> result;
  result.d_data_sp->fulfill(std::forward<Types2>(values)...);
  return result;
}

template <typename... Types>
template <typename... Types2>
Promise<Types2...> Promise<Types...>::reject(std::exception_ptr error) {
  Promise<Types2...> result;
  result.d_data_sp->reject(std::move(error));
  return result;
}

template <typename... Types>
Promise<Types...>::Promise()
    : d_data_sp(std::make_shared<PromiseState<Types...>>()) {}
}

#endif
