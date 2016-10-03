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

template <typename... Types>
class callable_placeholder {
    void operator()(Types...) const {}
};

template <typename F, typename... Types>
concept bool Resolver = requires(F f, Types... t)
{
    // Cannot use a lambda as in the following snippet because it would be in
    // an unevaluated context.
    //   resolver([](Type){}, [](std::exception_ptr){});
    // Instead we use a placeholder type.
    //
    // TODO: `callable_placeholder` is not sufficient. We also need to "verify"
    // that function pointers and other invokables work.

    f(callable_placeholder<Types...>(),
      callable_placeholder<std::exception_ptr>());
};

template <typename... Types>
class promise {
    static const std::size_t waiting_state   = 0;
    static const std::size_t fulfilled_state = 1;
    static const std::size_t rejected_state  = 2;

    struct data {
        bbp::variant<bbp::monostate, std::tuple<Types...>, std::exception_ptr>
            d_state;

        // TODO: Is this the best type for this vector to have, or is there
        // something better? Each function might get its own allocation
        // unfortunately.
        std::vector<std::pair<std::function<void(Types...)>,
                              std::function<void(std::exception_ptr)> > >
            d_childFutures;
    };

    // TODO: Think about whether or not this needs to be protected by a mutex.
    // Think about which operations can be called concurrently.
    std::shared_ptr<data> d_data;

  public:
    using types = std::tuple<Types...>;

    promise(Resolver<Types...> resolver)
    : d_data(std::make_shared<data>())
    {
        resolver(
            [d_data = this->d_data](Types... t) noexcept {
                // TODO: think about how I can move t into the state and then
                // use it
                // for the child future notifications.
                d_data->d_state.template emplace<fulfilled_state>(t...);
                for (auto&& pf : d_data->d_childFutures)
                    pf.first(t...);
            },
            [d_data = this->d_data](std::exception_ptr e) noexcept {
                d_data->d_state.template emplace<rejected_state>(e);
                for (auto&& pf : d_data->d_childFutures)
                    pf.second(e);
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

        if (bbp::get_if<waiting_state>(d_data->d_state)) {
            // TODO: think deeply about f and ef captures. These should
            // probably be
            // moved in. Maybe some tests with weird functions should be used.
            return promise<U>([this, f, ef](auto fulfill, auto reject) {
                d_data->d_childFutures.emplace_back(
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
        if (bbp::get_if<waiting_state>(d_data->d_state)) {
            // TODO: think deeply about f and ef captures. These should
            // probably be
            // moved in. Maybe some tests with weird functions should be used.
            return promise<>([this, f, ef](auto fulfill, auto reject) {
                d_data->d_childFutures.emplace_back(
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
        if (bbp::get_if<waiting_state>(d_data->d_state)) {
            return promise<>([this, f](auto fulfill, auto reject) {
                d_data->d_childFutures.emplace_back(
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

        if (bbp::get_if<waiting_state>(d_data->d_state)) {
            return promise<U>([this, f](auto fulfill, auto reject) {
                d_data->d_childFutures.emplace_back(
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

// TODO: Add support for a then result of a promise producing a promise of
// the internal type instead of a promise promise.
// TODO: clean up the test cases now that we have 'fulfilled' and 'rejected'
// static functions.
// TODO: Add cancelation support
// TODO: Add support for a then result of a tuple.
// TODO: Add support for a then result of a 'keep' which keeps the result
// value no matter what its type is.
// TODO: All function arguments should be by &&
// TODO: In the context of 'then', if 'ef's return value is convertable to 'f's
// return value, that should be okay.
// TODO: For this implementation to work properly, we are guarenteeing that
// the fulfil and reject functions passed to the resolver do not throw
// exceptions. To fix this, I need to ensure that my "try catch" block is over
// the call of the functions passed into then. The result then needs to be
// captured and used later.
// TODO: The common-type of the results of the 'f' and 'ef' functions should be
// used as the resulting promise's type  for the two arguent versions of
// 'then'.
// TODO: Consider adding a second 'rejected' function which uses the Types
// argument of the promise. Really, these things should be independent of the
// class template in a PromiseUtil.
}
