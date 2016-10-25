#include <bbp/ranges_concepts.h>  // bbp::Callable
#include <bbp/variant.h>
#include <exception>    // std::exception_ptr
#include <tuple>        // std::invoke, std::tuple
#include <type_traits>  // std::is_same, std::result_of, std::is_void
#include <vector>

#include <experimental/tuple>  // std::experimental::apply
#include <experimental/type_traits>  // std::experimental::is_void_v, std::experimental::is_same_v

namespace bbp {

// 'callable_placeholder<Types...>' is a type that satisifies
// 'Callable<Types...>'.
//
// Note that placeholders like this are required for concepts that require
// arbitrary types matching another concept in their definition. Also note that
// while this makes for a decent check it is not exhaustive (nor can it be).
template <typename... Types>
class callable_placeholder {
    void operator()(Types...) const {}
};

// Types that satisfy 'Resolver<Types...>' are callable with their first
// argument satisfying 'Callable<Types...>' and their second argument
// satisfying
// 'Callable<std::exception_ptr>'.
template <typename F, typename... Types>
concept bool Resolver = Callable<F,
                                 callable_placeholder<Types...>,
                                 callable_placeholder<std::exception_ptr> >;

// What follows is an alternate definition of the same concept.
//
// template <typename F, typename... Types>
// concept bool Resolver = requires(F f, Types... t)
// {
//     f(callable_placeholder<Types...>(),
//       callable_placeholder<std::exception_ptr>());
// };

// This class implements a value semantic type representing a heterogenius
// sequence of values or exception at some point in time.
//
// Note that this is a lot like a 'future', but with different constructors,
// simplified 'then' operations, and support for multiple fulfilled
// values.
template <typename... Types>
class promise {
    // Promises can be one of three states
    static constexpr std::size_t waiting_state = 0;  // Not fulfilled yet.

    static constexpr std::size_t fulfilled_state = 1;  // Fulfilled with a
                                                       // sequence of values.

    static constexpr std::size_t rejected_state = 2;  // Rejected with an
                                                      // exception.

    // This class is a value semantic type that implements the state stored by
    // the promise.
    struct data {
        // Some of the three states carry data.
        bbp::variant<
            // The waiting state includes a list of functions to be called when
            // fulfilment or rejection occurs.
            std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >,

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
    // 'Callable<Types...>' (resolve function) as its first and a
    // 'Callable<std::exception_ptr>' (reject function) as its second argument.
    // When the resolve function is called, this promise is fulfilled with its
    // arguments. When the reject function is called, this promise is rejected
    // with its argument. The behavior is undefined unless at most one of the
    // arguments to 'resolver' is ever called.
    //
    // Note that neither the reject nor resolve functions need be called from
    // within 'resolver'. 'resolver' could, for example, store these functions
    // elsewhere to be called at a later time.
    promise(Resolver<Types...> resolver)
    : d_data(std::make_shared<data>())
    {
        // Set 'fulfil' to the fulfilment function. Note that it, as well as
        // reject, keeps its own shared pointer to 'd_data'.
        auto fulfil = [d_data = this->d_data](Types... fulfillValues) noexcept
        {
            // Call all the waiting functions with the fulfill values.
            for (auto&& pf : bbp::get<waiting_state>(d_data->d_state))
                pf.first(fulfillValues...);

            // Move to the fulfilled state
            d_data->d_state.template emplace<fulfilled_state>(
                std::move(fulfillValues)...);
        };

        auto reject = [d_data = this->d_data](std::exception_ptr e) noexcept
        {
            // Call all the waiting functions with the exception.
            for (auto&& pf : bbp::get<waiting_state>(d_data->d_state))
                pf.second(e);

            // Move to the rejected state
            d_data->d_state.template emplace<rejected_state>(std::move(e));
        };

        resolver(std::move(fulfil), std::move(reject));
    }

    // Return a new promise that, upon the fulfilment of this promise, will be
    // fulfilled with the result of the specified 'fulfilledCont' function or,
    // upon reject of this promise, will be rejected with the same
    // 'std::exception_ptr'.
    //
    // Optionally specify a 'rejectedCont' function, which provides the newly
    // created promise with a fulfillment value in case this promise is
    // rejected.
    // 'fulfilledCont' called with arguments of type 'Types...' must have the
    // same return type as 'rejectedCont' when called with a
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
    // some
    //    types 'T1,T2,...', the result of this function will be of type
    //    'promise<T1,T2,...>'.
    // 4. If none of the above three conditions apply and the return type of
    //    'fulfilledCont' is 'T', then the result of this function will be of
    //    type 'promise<T>'.
    template <Callable<Types...>           FulfilledCont,
              Callable<std::exception_ptr> RejectedCont>
    promise<std::result_of_t<FulfilledCont(Types...)> >
    then(FulfilledCont fulfilledCont,
         RejectedCont  rejectedCont)  // Two-argument version of case #4
        requires(
            // non-void return type
            !std::experimental::is_void_v<
                std::result_of_t<FulfilledCont(Types...)> > &&

            // 'fulfill' and 'reject' have matching return values
            std::experimental::is_same_v<
                std::result_of_t<FulfilledCont(Types...)>,
                std::result_of_t<RejectedCont(std::exception_ptr)> >)
    {
        // TODO: left off here for initial code review documentation.
        using U = std::result_of_t<FulfilledCont(Types...)>;

        if (std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >
                *const waitingFunctions =
                    bbp::get_if<waiting_state>(d_data->d_state)) {
            // TODO: think deeply about f and ef captures. These should
            // probably be moved in. Maybe some tests with weird functions
            // should be used.
            return promise<U>([
                waitingFunctions,
                fulfilledCont = std::move(fulfilledCont),
                rejectedCont  = std::move(rejectedCont)
            ](auto fulfill, auto reject) mutable {
                waitingFunctions->emplace_back(
                    [
                      fulfilledCont = std::move(fulfilledCont),
                      fulfill,
                      reject
                    ](Types... t) {
                        try {
                            fulfill(fulfilledCont(t...));
                        }
                        catch (...) {
                            reject(std::current_exception());
                        }
                    },
                    [
                      rejectedCont = std::move(rejectedCont),
                      fulfill,
                      reject
                    ](std::exception_ptr e) {
                        try {
                            fulfill(rejectedCont(e));
                        }
                        catch (...) {
                            reject(std::current_exception());
                        }
                    });
            });
        }
        else if (std::tuple<Types...> *t =
                     bbp::get_if<fulfilled_state>(d_data->d_state)) {
            return promise<U>([ t, fulfilledCont = std::move(fulfilledCont) ](
                auto fulfill, auto reject) mutable {
                try {
                    fulfill(std::experimental::apply(fulfilledCont, *t));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            });
        }
        else {
            std::exception_ptr e = bbp::get<rejected_state>(d_data->d_state);
            return promise<U>([&e, rejectedCont = std::move(rejectedCont) ](
                auto fulfill, auto reject) mutable {
                try {
                    fulfill(rejectedCont(std::move(e)));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            });
        }
    }

    // Two-argument 'then' with void continuations.
    template <Callable<Types...>           FulfilledCont,
              Callable<std::exception_ptr> RejectedCont>
    requires(std::is_void<
             typename std::result_of<FulfilledCont(Types...)>::type>::value&&
                 std::is_same<
                     typename std::result_of<FulfilledCont(Types...)>::type,
                     typename std::result_of<
                         RejectedCont(std::exception_ptr)>::type>::value)
        promise<> then(FulfilledCont f, RejectedCont ef)
    {
        if (std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >
                *const waitingFunctions =
                    bbp::get_if<waiting_state>(d_data->d_state)) {
            // TODO: think deeply about f and ef captures. These should
            // probably be moved in. Maybe some tests with weird functions
            // should be used.
            return promise<>(
                [waitingFunctions, f, ef](auto fulfill, auto reject) {
                    waitingFunctions->emplace_back(
                        [f, fulfill, reject](Types... t) {
                            try {
                                f(t...);
                                fulfill();
                            }
                            catch (...) {
                                reject(std::current_exception());
                            }
                        },
                        [ef, fulfill, reject](std::exception_ptr e) {
                            try {
                                ef(e);
                                fulfill();
                            }
                            catch (...) {
                                reject(std::current_exception());
                            }
                        });
                });
        }
        else if (std::tuple<Types...> *t =
                     bbp::get_if<fulfilled_state>(d_data->d_state)) {
            return promise<>(
                [ t, f = std::move(f) ](auto fulfill, auto reject) mutable {
                    try {
                        std::experimental::apply(f, *t);
                        fulfill();
                    }
                    catch (...) {
                        reject(std::current_exception());
                    }
                });
        }
        else {
            std::exception_ptr e = bbp::get<rejected_state>(d_data->d_state);
            return promise<>(
                [&e, ef = std::move(ef) ](auto fulfill, auto reject) mutable {
                    try {
                        ef(std::move(e));
                        fulfill();
                    }
                    catch (...) {
                        reject(std::current_exception());
                    }
                });
        }
    }

    // One-argument 'then' with void continuation.
    template <Callable<Types...> FulfilledCont>
    requires std::is_void<
        typename std::result_of<FulfilledCont(Types...)>::type>::value
        promise<>
        then(FulfilledCont f)
    {
        if (std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >
                *const waitingFunctions =
                    bbp::get_if<waiting_state>(d_data->d_state)) {
            return promise<>([waitingFunctions, f](auto fulfill, auto reject) {
                waitingFunctions->emplace_back(
                    [f, fulfill, reject](Types... t) {
                        try {
                            f(t...);
                            fulfill();
                        }
                        catch (...) {
                            reject(std::current_exception());
                        }
                    },
                    [reject](std::exception_ptr e) { reject(e); });
            });
        }
        else if (std::tuple<Types...> *t =
                     bbp::get_if<fulfilled_state>(d_data->d_state)) {
            return promise<>(
                [ t, f = std::move(f) ](auto fulfill, auto reject) mutable {
                    try {
                        std::experimental::apply(f, *t);
                        fulfill();
                    }
                    catch (...) {
                        reject(std::current_exception());
                    }
                });
        }
        else {
            std::exception_ptr e = bbp::get<rejected_state>(d_data->d_state);
            return promise<>([&e](auto fulfill, auto reject) mutable {
                reject(std::move(e));
            });
        }
    }

    // One-argument 'then' with non-tuple, non-void continuation.
    //
    // Note that this function is semantically equivelent to
    // ```
    // then( f, [](std::exception_ptr e) -> U { std::rethrow_exception(e); }
    // ```
    // We directly implement it here to avoid the rethrow of the exception.
    //
    // TODO: Same comment applies to the previous function.
    template <Callable<Types...> FulfilledCont>
    requires !std::is_void<
        typename std::result_of<FulfilledCont(Types...)>::type>::value
        promise<typename std::result_of<FulfilledCont(Types...)>::type>
        then(FulfilledCont f)
    {
        typedef typename std::result_of<FulfilledCont(Types...)>::type U;

        if (std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >
                *const waitingFunctions =
                    bbp::get_if<waiting_state>(d_data->d_state)) {
            return promise<U>(
                [waitingFunctions, f](auto fulfill, auto reject) {
                    waitingFunctions->emplace_back(
                        [f, fulfill, reject](Types... t) {
                            try {
                                fulfill(f(t...));
                            }
                            catch (...) {
                                reject(std::current_exception());
                            }
                        },
                        [reject](std::exception_ptr e) { reject(e); });
                });
        }
        else if (std::tuple<Types...> *t =
                     bbp::get_if<fulfilled_state>(d_data->d_state)) {
            return promise<U>(
                [ t, f = std::move(f) ](auto fulfill, auto reject) mutable {
                    try {
                        fulfill(std::experimental::apply(f, *t));
                    }
                    catch (...) {
                        reject(std::current_exception());
                    }
                });
        }
        else {
            std::exception_ptr e = bbp::get<rejected_state>(d_data->d_state);
            return promise<U>([&e](auto fulfill, auto reject) mutable {
                reject(std::move(e));
            });
        }
    }

    // Return a promise with the specified types that is fulfilled with the
    // specified 'values'.
    template <typename...     Types2>
    static promise<Types2...> fulfilled(Types2&&... values)
    {
        promise<Types2...> result;
        result.d_data->d_state.template emplace<fulfilled_state>(
            std::move(values)...);
        return result;
    }

    // Return a promise with the specified types that is rejected with the
    // specified 'error'.
    template <typename...     Types2>
    static promise<Types2...> rejected(std::exception_ptr error)
    {
        promise<Types2...> result;
        result.d_data->d_state.template emplace<rejected_state>(
            std::move(error));
        return result;
    }

  private:
    // Create a new 'promise' object in the waiting state. It is never
    // fulfilled.
    promise()
    : d_data(std::make_shared<data>())
    {
    }
};
}
