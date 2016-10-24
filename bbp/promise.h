#include <bbp/variant.h>
#include <exception>    // std::exception_ptr
#include <tuple>        // std::invoke, std::tuple
#include <type_traits>  // std::is_same, std::result_of, std::is_void
#include <vector>

#include <experimental/tuple>  // std::experimental::apply

namespace bbp {

// Note that this is an abridged implementation of the like named 'Callable'
// concept in the Range's TS.
template <typename F, typename... Types>
concept bool Callable = requires(F f, Types... t)
{
    f(t...);
};

// Because one cannot write a concept that uses an "instance" of another
// concept, I use placeholders to kind-of simulate that idea. This class acts
// like an arbitrary callable that matches the above Callable concept.
template <typename... Types>
class callable_placeholder {
    void operator()(Types...) const {}
};

// Types that are "Resolvers<Types...>" have instances that take in callables
// as their first parameters. These callables take in 'Types...' in the call
// operator. This is kinda hard to explain.
template <typename F, typename... Types>
concept bool Resolver = requires(F f, Types... t)
{
    // Note that `callable_placeholder` is not sufficient. We also need to
    // check that function pointers and other invokables work.
    f(callable_placeholder<Types...>(),
      callable_placeholder<std::exception_ptr>());
};

// Here's our promise object. It is like a 'future' except we have different
// constructors, different then operations, and support multiple fulfilled
// types.
template <typename... Types>
class promise {
    // The promise can be in tese three states
    static const std::size_t waiting_state   = 0;
    static const std::size_t fulfilled_state = 1;
    static const std::size_t rejected_state  = 2;

    struct data {
        // Some of the three states carry data.
        bbp::variant<
            // The waiting state carries with it a list of functions that need
            // to be called when a value is actually present.
            std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >,

            // The fulfilled state carries with it a tuple of the fulfilled
            // values.
            std::tuple<Types...>,

            // The rejected state carries with it a 'std::exception_ptr'.
            std::exception_ptr>
            d_state;
    };

    // Promises can be copied and there is no semantic problem with this since
    // they have no mutating members. Well, at least until a 'cancel' operation
    // is implemented anyway.
    std::shared_ptr<data> d_data;

  public:

    // TODO: left off here for initial code review documentation.

    promise(Resolver<Types...> resolver)
    : d_data(std::make_shared<data>())
    {
        resolver(
            [d_data = this->d_data](Types... t) noexcept {
                // Call all the waiting functions with the fulfill value.
                for (auto&& pf : bbp::get<waiting_state>(d_data->d_state))
                    pf.first(t...);

                // Move to the fulfilled state
                d_data->d_state.template emplace<fulfilled_state>(
                    std::move(t)...);
            },
            [d_data = this->d_data](std::exception_ptr e) noexcept {
                // Call all the waiting functions with the exception.
                for (auto&& pf : bbp::get<waiting_state>(d_data->d_state))
                    pf.second(e);

                // Move to the rejected state
                d_data->d_state.template emplace<rejected_state>(std::move(e));
            });
    }

    // Two-argument 'then' with non-void, non-tuple continuations.
    template <Callable<Types...>           ValueContinuation,
              Callable<std::exception_ptr> ErrorContinuation>
    // TODO: seems like this should be cleaner.
    requires(!std::is_void<typename std::result_of<
                 ValueContinuation(Types...)>::type>::value &&

             std::is_same<
                 typename std::result_of<ValueContinuation(Types...)>::type,
                 typename std::result_of<
                     ErrorContinuation(std::exception_ptr)>::type>::value)
        promise<typename std::result_of<ValueContinuation(
            Types...)>::type> then(ValueContinuation f, ErrorContinuation ef)
    {
        typedef typename std::result_of<ValueContinuation(Types...)>::type U;

        if (std::vector<std::pair<std::function<void(Types...)>,
                                  std::function<void(std::exception_ptr)> > >
                *const waitingFunctions =
                    bbp::get_if<waiting_state>(d_data->d_state)) {
            // TODO: think deeply about f and ef captures. These should
            // probably be moved in. Maybe some tests with weird functions
            // should be used.
            return promise<U>(
                [waitingFunctions, f, ef](auto fulfill, auto reject) {
                    waitingFunctions->emplace_back(
                        [f, fulfill, reject](Types... t) {
                            try {
                                fulfill(f(t...));
                            }
                            catch (...) {
                                reject(std::current_exception());
                            }
                        },
                        [ef, fulfill, reject](std::exception_ptr e) {
                            try {
                                fulfill(ef(e));
                            }
                            catch (...) {
                                reject(std::current_exception());
                            }
                        });
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
            return promise<U>(
                [&e, ef = std::move(ef) ](auto fulfill, auto reject) mutable {
                    try {
                        fulfill(ef(std::move(e)));
                    }
                    catch (...) {
                        reject(std::current_exception());
                    }
                });
        }
    }

    // Two-argument 'then' with void continuations.
    template <Callable<Types...>           ValueContinuation,
              Callable<std::exception_ptr> ErrorContinuation>
    requires(
        std::is_void<typename std::result_of<
            ValueContinuation(Types...)>::type>::value&& std::
            is_same<typename std::result_of<ValueContinuation(Types...)>::type,
                    typename std::result_of<
                        ErrorContinuation(std::exception_ptr)>::type>::value)
        promise<> then(ValueContinuation f, ErrorContinuation ef)
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
    template <Callable<Types...> ValueContinuation>
    requires std::is_void<
        typename std::result_of<ValueContinuation(Types...)>::type>::value
        promise<>
        then(ValueContinuation f)
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
    template <Callable<Types...> ValueContinuation>
    requires !std::is_void<
        typename std::result_of<ValueContinuation(Types...)>::type>::value
        promise<typename std::result_of<ValueContinuation(Types...)>::type>
        then(ValueContinuation f)
    {
        typedef typename std::result_of<ValueContinuation(Types...)>::type U;

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
