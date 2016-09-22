#include <bbp/variant.h>
#include <exception>   // std::exception_ptr
#include <type_traits> // std::is_same, std::result_of
#include <vector>

namespace bbp {

// Note that this is an abridged implementation of the like named 'Callable'
// concept in the Range's TS.
template <typename F, typename Type>
concept bool Callable = requires(F f, Type t) {
  f(t);
};

template <typename Type> class callable_placeholder {
  void operator()(Type) const {}
};

class reject_placeholder {
  void operator()(std::exception_ptr);
};

template <typename F, typename Type>
concept bool Resolver = requires(F f, Type t) {
  // Cannot use a lambda as in the following snippet because it would be in
  // an unevaluated context.
  //   resolver([](Type){}, [](std::exception_ptr){});
  // Instead we use a placeholder type.
  //
  // TODO: `callable_placeholder` is not sufficient. We also need to "verify"
  // that function pointers and other invokables work.
  f(callable_placeholder<Type>(), callable_placeholder<std::exception_ptr>());
};

template <typename Type> class promise {

  static const std::size_t waiting_state = 0;
  static const std::size_t fulfilled_state = 1;
  static const std::size_t rejected_state = 2;

  struct data {
    bbp::variant<bbp::monostate, Type, std::exception_ptr> d_state;

    // TODO: Is this the best type for this vector to have, or is there
    // something better? Each function might get its own allocation
    // unfortunately.
    std::vector<std::pair<std::function<void(Type)>,
                          std::function<void(std::exception_ptr)>>>
        d_childFutures;
  };

  // TODO: Think about whether or not this needs to be protected by a mutex.
  // Think about which operations can be called concurrently.
  std::shared_ptr<data> d_data;

public:
  using type = Type;

  promise(Resolver<Type> resolver) : d_data(std::make_shared<data>()) {
    resolver(
        [d_data = this->d_data](Type t) noexcept {
          // TODO: think about how I can move t into the state and then use it
          // for the child future notifications.
          d_data->d_state.template emplace<fulfilled_state>(t);
          for (auto &&pf : d_data->d_childFutures)
            pf.first(t);
        },
        [d_data = this->d_data](std::exception_ptr e) noexcept {
          d_data->d_state.template emplace<rejected_state>(e);
          for (auto &&pf : d_data->d_childFutures)
            pf.second(e);
        });
  }
  template <Callable<Type> ValueContinuation,
            Callable<std::exception_ptr> ErrorContinuation>
  // TODO: seems like this should be cleaner.
  requires std::is_same<typename std::result_of<ValueContinuation(Type)>::type,
                        typename std::result_of<
                            ErrorContinuation(std::exception_ptr)>::type>::value
      promise<typename std::result_of<ValueContinuation(Type)>::type>
      then(ValueContinuation f, ErrorContinuation ef) {

    typedef typename std::result_of<ValueContinuation(Type)>::type U;

    if (bbp::get_if<waiting_state>(d_data->d_state)) {
      // TODO: think deeply about f and ef captures. These should probably be
      // moved in. Maybe some tests with weird functions should be used.
      return promise<U>([this, f, ef](auto fulfill, auto reject) {
        d_data->d_childFutures.emplace_back(
            [f, fulfill, reject](Type t) {
              try {
                fulfill(f(t));
              } catch (...) {
                reject(std::current_exception());
              }
            },
            [ef, fulfill, reject](std::exception_ptr e) {
              try {
                fulfill(ef(e));
              } catch (...) {
                reject(std::current_exception());
              }
            });
      });
    } else if (Type *t = bbp::get_if<fulfilled_state>(d_data->d_state)) {
      return promise<U>(
          [ t, f = std::move(f) ](auto fulfill, auto reject) mutable {
            try {
              fulfill(f(*t));
            } catch (...) {
              reject(std::current_exception());
            }
          });
    } else {
      std::exception_ptr e = bbp::get<rejected_state>(d_data->d_state);
      return promise<U>(
          [&e, ef = std::move(ef) ](auto fulfill, auto reject) mutable {
            try {
              fulfill(ef(std::move(e)));
            } catch (...) {
              reject(std::current_exception());
            }
          });
    }
  }
  template <Callable<Type> ValueContinuation>
  promise<typename std::result_of<ValueContinuation(Type)>::type>
  then(ValueContinuation f) {
    // Note that this function is semantically equivelent to
    // ```
    // then( f, [](std::exception_ptr e) -> U { std::rethrow_exception(e); }
    // ```
    // We directly implement it here to avoid the rethrow of the exception.

    typedef typename std::result_of<ValueContinuation(Type)>::type U;

    if (bbp::get_if<waiting_state>(d_data->d_state)) {
      return promise<U>([this, f](auto fulfill, auto reject) {
        d_data->d_childFutures.emplace_back(
            [f, fulfill, reject](Type t) {
              try {
                fulfill(f(t));
              } catch (...) {
                reject(std::current_exception());
              }
            },
            [reject](std::exception_ptr e) { reject(e); });
      });
    } else if (Type *t = bbp::get_if<fulfilled_state>(d_data->d_state)) {
      return promise<U>(
          [ t, f = std::move(f) ](auto fulfill, auto reject) mutable {
            try {
              fulfill(f(*t));
            } catch (...) {
              reject(std::current_exception());
            }
          });
    } else {
      std::exception_ptr e = bbp::get<rejected_state>(d_data->d_state);
      return promise<U>(
          [&e](auto fulfill, auto reject) mutable { reject(std::move(e)); });
    }
  }
};

// TODO: Add empty promises.
// TODO: Add variadic promises.
// TODO: In the context of 'then', if 'ef's return value is convertable to 'f's
// return value, that should be okay.
// TODO: For this implementation to work properly, we are guarenteeing that
// the fulfil and reject functions passed to the resolver do not throw
// exceptions. To fix this, I need to ensure that my "try catch" block is over
// the call of the functions passed into then. The result then needs to be
// captured and used later.
}
