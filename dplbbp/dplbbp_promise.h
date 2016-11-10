#ifndef BBP_PROMISE_INCLUDED
#define BBP_PROMISE_INCLUDED

#include <dplmrts_invocable.h>
#include <dplmrts_invocablearchetype.h>
#include <dplbbp_anypromise.h>
#include <dplm17_variant.h>
#include <dplmrts_anytuple.h>

#include <experimental/tuple> // std::experimental::apply
#include <experimental/type_traits> // std::experimental::is_void_v, std::experimental::is_same_v

#include <exception> // std::exception_ptr
#include <functional> // std::function, std::invoke
#include <iostream>
#include <tuple>       // std::invoke, std::tuple
#include <type_traits> // std::is_same, std::result_of, std::is_void
#include <vector>

namespace dplbbp {

// Types that satisfy 'Resolver<Types...>' are callable with their first
// argument satisfying 'dplmrts::Invocable<Types...>' and their second argument
// satisfying 'dplmrts::Invocable<std::exception_ptr>'.
template <typename F, typename... Types>
concept bool Resolver = dplmrts::Invocable<F, dplmrts::InvocableArchetype<Types...>,
                                 dplmrts::InvocableArchetype<std::exception_ptr>>;

// What follows is an alternate definition of the same concept.
//
// template <typename F, typename... Types>
// concept bool Resolver = requires(F f, Types... t)
// {
//     f(dplmrts::InvocableArchetype<Types...>(),
//       dplmrts::InvocableArchetype<std::exception_ptr>());
// };

template <typename T> struct PromiseFromTuple {};

template <typename... T> struct PromiseFromTuple<std::tuple<T...>> {
  using type = dplbbp::promise<T...>;
};

template <typename Tuple>
using PromiseFromTuple_t = typename PromiseFromTuple<Tuple>::type;

// This class implements a value semantic type representing a heterogenius
// sequence of values or exception at some point in time.
//
// Note that this is a lot like a 'future', but with different constructors,
// simplified 'then' operations, and support for multiple fulfilled
// values.
template <typename... Types> class promise {
  // Promises can be one of three states
  static constexpr std::size_t waiting_state = 0; // Not fulfilled yet.

  static constexpr std::size_t fulfilled_state = 1; // Fulfilled with a
                                                    // sequence of values.

  static constexpr std::size_t rejected_state = 2; // Rejected with an
                                                   // exception.

  // This class is a value semantic type that implements the state stored by
  // the promise.
  struct data {
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

  // Promises may be copied. There are no semantic problems with this since
  // they have no mutating members. Well, at least until a 'cancel' operation
  // is implemented anyway.
  std::shared_ptr<data> d_data;

public:
  // Create a new 'promise' object based on the specified 'resolver'.
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
  promise(Resolver<Types...> resolver);

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
  //    function will be of type 'promise<>'.
  // 2. If the return type of 'fulfilledCont' is 'tuple<T1,T2,...>' for some
  //    types 'T1,T2,...', the result of this function will be of type
  //    'promise<T1,T2,...>'.
  // 3. If the return type of 'fulfilledCont' is 'promise<T1,T2,...>' for
  //    some types 'T1,T2,...', the result of this function will be of type
  //    'promise<T1,T2,...>'.
  // 4. If none of the above three conditions apply and the return type of
  //    'fulfilledCont' is 'T', then the result of this function will be of
  //    type 'promise<T>'.

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
  promise<> then(FulfilledCont fulfilledCont,
                 RejectedCont rejectedCont) // Two-argument version of case #1
      requires
      // void return type
      std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
  promise<> then(FulfilledCont fulfilledCont) // One-argument version of case #1
      requires
      // void return type
      std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
  PromiseFromTuple_t<std::result_of_t<FulfilledCont(Types...)>>
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
  PromiseFromTuple_t<std::result_of_t<FulfilledCont(Types...)>>
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
      dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
  std::result_of_t<FulfilledCont(Types...)>
  then(FulfilledCont fulfilledCont) // One-argument version of case #3
      requires
      // promise return type
      dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont,
            dplmrts::Invocable<std::exception_ptr> RejectedCont>
      promise<std::result_of_t<FulfilledCont(Types...)>>
      then(FulfilledCont fulfilledCont,
           RejectedCont rejectedCont) // Two-argument version of case #4
      requires
      // non-void return type
      !std::experimental::is_void_v<
          std::result_of_t<FulfilledCont(Types...)>> &&

      // non-tuple return type
      !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

      // non-promise return type
      !dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

      // 'fulfilledCont' and 'rejectedCont' have matching return values
      std::experimental::is_same_v<
          std::result_of_t<FulfilledCont(Types...)>,
          std::result_of_t<RejectedCont(std::exception_ptr)>>;

  template <dplmrts::Invocable<Types...> FulfilledCont>
      promise<std::result_of_t<FulfilledCont(Types...)>>
      then(FulfilledCont fulfilledCont) // One-argument version of case #4
      requires
      // non-void return type
      !std::experimental::is_void_v<
          std::result_of_t<FulfilledCont(Types...)>> &&

      // non-tuple return type
      !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

      // non-promise return type
      !dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>>;

  // Return a promise with the specified types that is fulfilled with the
  // specified 'values'.
  template <typename... Types2>
  static promise<Types2...> fulfill(Types2 &&... values);

  // Return a promise with the specified types that is rejected with the
  // specified 'error'.
  template <typename... Types2>
  static promise<Types2...> reject(std::exception_ptr error);

private:
  // Create a new 'promise' object in the waiting state. It is never
  // fulfilled.
  promise();
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... Types>
promise<Types...>::promise(Resolver<Types...> resolver) : d_data(std::make_shared<data>())
{
  // Set 'fulfil' to the fulfilment function. Note that it, as well as
  // reject, keeps its own shared pointer to 'd_data'.
  auto fulfil = [d_data = this->d_data](Types... fulfillValues) noexcept {
    // Call all the waiting functions with the fulfill values.
    for (auto &&pf : dplm17::get<waiting_state>(d_data->d_state))
      pf.first(fulfillValues...);

    // Move to the fulfilled state
    d_data->d_state.template emplace<fulfilled_state>(
        std::move(fulfillValues)...);
  };

  auto reject = [d_data = this->d_data](std::exception_ptr e) noexcept {
    // Call all the waiting functions with the exception.
    for (auto &&pf : dplm17::get<waiting_state>(d_data->d_state))
      pf.second(e);

    // Move to the rejected state
    d_data->d_state.template emplace<rejected_state>(std::move(e));
  };

  std::invoke(resolver, std::move(fulfil), std::move(reject));
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
promise<> promise<Types...>::then(FulfilledCont fulfilledCont,
               RejectedCont rejectedCont) // Two-argument version of case #1
    requires
    // void return type
    std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    return promise<>([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont),
      rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
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
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return promise<>([
      fulfilledValues, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      try {
        std::experimental::apply(std::move(fulfilledCont), *fulfilledValues);
        fulfill();
      } catch (...) {
        reject(std::current_exception());
      }
    });
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return promise<>([
      &rejectedException, rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      try {
        std::invoke(std::move(rejectedCont), rejectedException);
        fulfill();
      } catch (...) {
        reject(std::current_exception());
      }
    });
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
promise<> promise<Types...>::then(FulfilledCont fulfilledCont) // One-argument version of case #1
    requires
    // void return type
    std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)>> {
  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    return promise<>([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
          [ fulfilledCont = std::move(fulfilledCont), fulfill,
            reject ](Types... t) {
            try {
              std::invoke(std::move(fulfilledCont), t...);
              fulfill();
            } catch (...) {
              reject(std::current_exception());
            }
          },
          [reject](std::exception_ptr e) { reject(e); });
    });
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return promise<>([
      fulfilledValues, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      try {
        std::experimental::apply(std::move(fulfilledCont), *fulfilledValues);
        fulfill();
      } catch (...) {
        reject(std::current_exception());
      }
    });
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return promise<>([&rejectedException](auto fulfill, auto reject) {
      reject(rejectedException);
    });
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
PromiseFromTuple_t<std::result_of_t<FulfilledCont(Types...)>>
promise<Types...>::then(FulfilledCont fulfilledCont,
     RejectedCont rejectedCont) // Two-argument version of case #2
    requires
    // tuple return type
    dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  using Result =
      PromiseFromTuple_t<std::result_of_t<FulfilledCont(Types...)>>;

  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    return Result([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont),
      rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
          [ fulfilledCont = std::move(fulfilledCont), fulfill,
            reject ](Types... t) mutable {
            try {
              std::experimental::apply(fulfill,
                                       std::move(fulfilledCont)(t...));
            } catch (...) {
              reject(std::current_exception());
            }
          },
          [ rejectedCont = std::move(rejectedCont), fulfill,
            reject ](std::exception_ptr e) mutable {
            try {
              std::experimental::apply(fulfill, std::invoke(std::move(rejectedCont), e));
            } catch (...) {
              reject(std::current_exception());
            }
          });
    });
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return Result([
      fulfilledValues, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      try {
        std::experimental::apply(
            fulfill, std::experimental::apply(std::move(fulfilledCont),
                                              *fulfilledValues));
      } catch (...) {
        reject(std::current_exception());
      }
    });
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return Result([
      &rejectedException, rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      try {
        std::experimental::apply(fulfill,
                                 std::invoke(std::move(rejectedCont), rejectedException));
      } catch (...) {
        reject(std::current_exception());
      }
    });
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
PromiseFromTuple_t<std::result_of_t<FulfilledCont(Types...)>>
promise<Types...>::then(FulfilledCont fulfilledCont) // One-argument version of case #2
    requires
    // tuple return type
    dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> {
  using Result =
      PromiseFromTuple_t<std::result_of_t<FulfilledCont(Types...)>>;

  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    return Result([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
          [ fulfilledCont = std::move(fulfilledCont), fulfill,
            reject ](Types... t) mutable {
            try {
              std::experimental::apply(fulfill,
                                       std::invoke(std::move(fulfilledCont),t...));
            } catch (...) {
              reject(std::current_exception());
            }
          },
          [reject](std::exception_ptr e) { reject(e); });
    });
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return Result([
      fulfilledValues, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      try {
        std::experimental::apply(
            fulfill, std::experimental::apply(std::move(fulfilledCont),
                                              *fulfilledValues));
      } catch (...) {
        reject(std::current_exception());
      }
    });
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return Result([&rejectedException](auto fulfill, auto reject) mutable {
      reject(rejectedException);
    });
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
std::result_of_t<FulfilledCont(Types...)>
promise<Types...>::then(FulfilledCont fulfilledCont,
     RejectedCont rejectedCont) // Two-argument version of case #3
    requires
    // promise return type
    dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  using Result = std::result_of_t<FulfilledCont(Types...)>;

  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    // Build a promise that is fulfilled or rejected based on an inner
    // promise.
    return Result([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont),
      rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
          [ fulfilledCont = std::move(fulfilledCont), fulfill,
            reject ](Types... t) mutable {
            try {
              Result innerPromise = std::invoke(std::move(fulfilledCont), t...);

              if (auto *const waitingFunctions = dplm17::get_if<waiting_state>(
                      innerPromise.d_data->d_state)) {
                waitingFunctions->emplace_back(fulfill, reject);
              } else if (auto const *const fulfilledValues =
                             dplm17::get_if<fulfilled_state>(
                                 innerPromise.d_data->d_state)) {
                std::experimental::apply(std::move(fulfill),
                                         (*fulfilledValues));
              } else {
                const std::exception_ptr &rejectedException =
                    dplm17::get<rejected_state>(innerPromise.d_data->d_state);
                reject(rejectedException);
              }
            } catch (...) {
              reject(std::current_exception());
            }
          },
          [ rejectedCont = std::move(rejectedCont), fulfill,
            reject ](const std::exception_ptr &e) {
            try {
              Result innerPromise = std::invoke(std::move(rejectedCont), e);

              if (auto *const waitingFunctions = dplm17::get_if<waiting_state>(
                      innerPromise.d_data->d_state)) {
                waitingFunctions->emplace_back(fulfill, reject);
              } else if (auto const *const fulfilledValues =
                             dplm17::get_if<fulfilled_state>(
                                 innerPromise.d_data->d_state)) {
                std::experimental::apply(std::move(fulfill),
                                         (*fulfilledValues));
              } else {
                const std::exception_ptr &rejectedException =
                    dplm17::get<rejected_state>(innerPromise.d_data->d_state);
                reject(rejectedException);
              }
            } catch (...) {
              reject(std::current_exception());
            }
          });
    });
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return std::experimental::apply(std::move(fulfilledCont),
                                    (*fulfilledValues));
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return std::invoke(std::move(rejectedCont), rejectedException);
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
std::result_of_t<FulfilledCont(Types...)>
promise<Types...>::then(FulfilledCont fulfilledCont) // One-argument version of case #3
    requires
    // promise return type
    dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> {
  using Result = std::result_of_t<FulfilledCont(Types...)>;

  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    // Build a promise that is fulfilled or rejected based on an inner
    // promise.
    return Result([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
          [ fulfilledCont = std::move(fulfilledCont), fulfill,
            reject ](Types... t) mutable {
            try {
              Result innerPromise = std::invoke(std::move(fulfilledCont), t...);

              if (auto *const waitingFunctions = dplm17::get_if<waiting_state>(
                      innerPromise.d_data->d_state)) {
                waitingFunctions->emplace_back(fulfill, reject);
              } else if (auto const *const fulfilledValues =
                             dplm17::get_if<fulfilled_state>(
                                 innerPromise.d_data->d_state)) {
                std::experimental::apply(std::move(fulfill),
                                         (*fulfilledValues));
              } else {
                const std::exception_ptr &rejectedException =
                    dplm17::get<rejected_state>(innerPromise.d_data->d_state);
                reject(rejectedException);
              }
            } catch (...) {
              reject(std::current_exception());
            }
          },
          [reject](std::exception_ptr e) { reject(e); });
    });
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return std::experimental::apply(std::move(fulfilledCont),
                                    (*fulfilledValues));
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return Result([&rejectedException](auto fulfill, auto reject) mutable {
      reject(rejectedException);
    });
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
    promise<std::result_of_t<FulfilledCont(Types...)>>
    promise<Types...>::then(FulfilledCont fulfilledCont,
         RejectedCont rejectedCont) // Two-argument version of case #4
    requires
    // non-void return type
    !std::experimental::is_void_v<
        std::result_of_t<FulfilledCont(Types...)>> &&

    // non-tuple return type
    !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

    // non-promise return type
    !dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)>> {
  using U = std::result_of_t<FulfilledCont(Types...)>;

  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    return promise<U>([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont),
      rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
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
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return promise<U>([
      fulfilledValues, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      try {
        fulfill(std::experimental::apply(std::move(fulfilledCont),
                                         *fulfilledValues));
      } catch (...) {
        reject(std::current_exception());
      }
    });
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return promise<U>([
      &rejectedException, rejectedCont = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
      try {
        fulfill(std::invoke(std::move(rejectedCont), rejectedException));
      } catch (...) {
        reject(std::current_exception());
      }
    });
  }
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
    promise<std::result_of_t<FulfilledCont(Types...)>>
    promise<Types...>::then(FulfilledCont fulfilledCont) // One-argument version of case #4
    requires
    // non-void return type
    !std::experimental::is_void_v<
        std::result_of_t<FulfilledCont(Types...)>> &&

    // non-tuple return type
    !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)>> &&

    // non-promise return type
    !dplbbp::AnyPromise<std::result_of_t<FulfilledCont(Types...)>> {
  using U = std::result_of_t<FulfilledCont(Types...)>;

  if (std::vector<std::pair<std::function<void(Types...)>,
                            std::function<void(std::exception_ptr)>>>
          *const waitingFunctions =
              dplm17::get_if<waiting_state>(d_data->d_state)) {
    return promise<U>([
      waitingFunctions, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      waitingFunctions->emplace_back(
          [ fulfilledCont = std::move(fulfilledCont), fulfill,
            reject ](Types... t) mutable {
            try {
              fulfill(std::invoke(std::move(fulfilledCont), t...));
            } catch (...) {
              reject(std::current_exception());
            }
          },
          [reject](std::exception_ptr e) { reject(e); });
    });
  } else if (std::tuple<Types...> const *const fulfilledValues =
                 dplm17::get_if<fulfilled_state>(d_data->d_state)) {
    return promise<U>([
      fulfilledValues, fulfilledCont = std::move(fulfilledCont)
    ](auto fulfill, auto reject) mutable {
      try {
        fulfill(std::experimental::apply(std::move(fulfilledCont),
                                         *fulfilledValues));
      } catch (...) {
        reject(std::current_exception());
      }
    });
  } else {
    const std::exception_ptr &rejectedException =
        dplm17::get<rejected_state>(d_data->d_state);
    return promise<U>([&rejectedException](
        auto fulfill, auto reject) mutable { reject(rejectedException); });
  }
}

template <typename... Types>
template <typename... Types2>
promise<Types2...> promise<Types...>::fulfill(Types2 &&... values) {
  promise<Types2...> result;
  result.d_data->d_state.template emplace<fulfilled_state>(
      std::move(values)...);
  return result;
}

template <typename... Types>
template <typename... Types2>
promise<Types2...> promise<Types...>::reject(std::exception_ptr error) {
  promise<Types2...> result;
  result.d_data->d_state.template emplace<rejected_state>(std::move(error));
  return result;
}

template <typename... Types>
promise<Types...>::promise() : d_data(std::make_shared<data>()) {}
}

#endif
