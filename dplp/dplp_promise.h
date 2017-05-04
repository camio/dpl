#ifndef INCLUDED_DPLP_PROMISE
#define INCLUDED_DPLP_PROMISE

//@PURPOSE: Provide a template representing asynchronous values.
//
//@CLASSES:
//  dplp::Promise: an asynchronous value template
//
//@DESCRIPTION: This component provides a single class, 'dplp::Promise', that
// represents asynchronous values. An asynchronous value is a value that may be
// resolved (aka. set) by another thread or operation. The value can only be
// set once and whether or not it is set is largely hidden by the interface.
// Promises are a basic building block for asynchronous applications.
//
///Usage
///-----
// This section illustrates intended use of this component.
//
///Example 1: Create a Promise
///- - - - - - - - - - - - - -
// The promise constructor takes in a single argument, called a resolver, which
// is a two argument invokable object. The first argument of the resolver is a
// function called 'fulfill' and resolver's second argument a function called
// 'reject'. 'fulfill' and 'reject' are used to set the promise's value. In the
// following snippet, we create an 'int' promise that has value 3.
//
//..
//  dplp::Promise<int> p([](auto fulfill, auto reject) {
//      fulfill(3);
//  });
//..
//
// We consider 'p' to be "fulfilled" with value '3'.
//
// Promises can also be "rejected" which means that instead of carrying a
// value, they hold an 'std::exception_ptr' object. In the following snippet,
// we set 'pError' to be "rejected" with a 'std::runtime_error' exception.
//
//..
//  dplp::Promise<int> p([](auto fulfill, auto reject) {
//      reject(std::make_exception_ptr(std::runtime_error("Error")));
//  });
//..
//
// Note that for any given promise, only one of 'fulfil' or 'reject' may be
// called and only one time at that. In other words, a promise is only
// fulfilled or rejected once.
//
///Example 2: Delayed Resolution
///- - - - - - - - - - - - - - -
// Promises need not call the 'fulfill' or 'reject' functions. In this snippet
// the promise 'never' never has 'fulfill' or 'reject' called.
//..
//  dplp::Promise<int> never([](auto fulfill, auto reject) {});
//..
// We say that 'never' is perpetually unresolved.
//
// More interestingly, however, the resolver can stow away the 'fulfill' and
// 'reject' functions to be called at another time.
//
//..
//  std::function<void (int)> myFulfill;
//  std::function<void (std::exception_ptr)> myReject;
//
//  dplp::Promise<int> q([](auto fulfill, auto reject) {
//      myFulfill = fulfill;
//      myReject = reject;
//  });
//..
// After 'q''s construction we say it is "unresolved". If the 'myFulfill'
// function gets called later on, we'd say at that point that 'q' became
// resolved, or more specifically, fulfilled. If, alternatively, the 'myReject'
// function gets called later on, we'd say at that point that 'q' became
// resolved, or more specifically, rejected.
//
// 'myFulfill' and 'myReject' can be called from any thread at any time.
//
///Example 2: Wrapping an Asynchronous Function
///- - - - - - - - - - - - - - - - - - - - - -
// Frequently preexisting libraries present an asynchronous interface where
// callbacks are installed and later called when a particular task is
// accomplished. Consider the following two functions:
//..
//  void receiveMessage(
//          std::function<void (std::error_code, std::string)> callback);
//      // Listen for a message and call 'callback' when the operation is
//      // complete with a 'std::error_code' indicating success and the
//      // retrieved message, if any.
//
//  void sendMessage(const std::string&                    message,
//                   std::function<void (std::error_code)> callback);
//      // Send the specified 'message'. Asynchronously call 'callback' when
//      // the operation is complete with a 'std::error_code' indicating
//      // success or failure.
//..
// Each of these functions can be simplified by wrapping them in a promise
// interface. Lets take a look at the signature of the wrapped interface.
//..
//  dplp::Promise<std::string> receiveMessageP();
//      // Listen for a message and return a promise that will be fulfilled
//      // with the result.
//
//  dplp::Promise<> sendMessageP(const std::string& message);
//      // Send the specified 'message'. Return a promise that is fulfilled
//      // when the operation is complete.
//..
// The first thing to note is that errors are not mentioned. The fact that the
// returned promise could end up in a rejected state is omited in a similar way
// to how exception documentation is omitted. If an asynchronous function
// cannot meet its post-conditions, it's returned promise is presumed to end up
// in a rejected state.
//
// The second thing to notice is that the reasult of 'sendMessageP' is
// 'dplp::Promise<>'. This is a promise that carries no particular value if it
// is resolved. We call these empty promises. Their primary purpose is to
// communicate when something happens (or if it didn't happen).
//
// 'receiveMessageP' and 'sendMessageP' are implemented in terms of
// 'receiveMessage' and 'sendMessage'.
//..
//  dplp::Promise<std::string> receiveMessageP()
//  {
//      return dplp::Promise<std::string>([](auto resolve, auto reject) {
//          receiveMessage([=](std::error_code ec, std::string msg) {
//              if(ec)
//                  reject(std::make_exception_ptr(std::system_error(ec)));
//              else
//                  fulfill(msg);
//          });
//      });
//  }
//..
// There are two lambdas in this example. The outer lambda is the resolver for
// the promise, which calls the wrapped function. The pattern of calling
// the wrapped function from within the resolver is common. The inner lambda is
// the callback for the wrapped function. Note that the callback captures
// 'resolve' and 'reject' so they can be called once the wrapped function
// completes its asynchronous operation.
//
// 'sendMessageP' is implemented in a similar way.
//..
//  dplp::Promise<> sendMessageP(const std::string& message) {
//      return dplp::Promise<>([&](auto resolve, auto reject) {
//          sendMessage(message, [=](std::error_code ec) {
//              if(ec)
//                  reject(std::make_exception_ptr(std::system_error(ec)));
//              else
//                  fulfill();
//          });
//      });
//  }
//..
// Note that the lambda for the resolver uses a '&' capture default. This is
// safe (ie. there's no danger of dangling references) because the resolver is
// called immediately in the promise constuctor.
//
// The other thing to note is that the 'fulfill' function for empty promises
// takes in no arguments.
//
///Example 3: Build promises from other promises
///- - - - - - - - - - - - - - - - - - - - - - -
// While the promise constructor is frequently used in lower-level components
// and for wrapping other asynchonous libraries, the most common way to create
// promises is build them based on other promises. This is accomplished with
// the 'then' member function.
//
// Say we want to write a function 'recieveIntP' which receives an int and is
// built off of 'receiveMessageP' from above. 'recieveIntP' should parse the
// received message as an 'int' and return an 'int' promise. This is
// implemented as follows.
//..
//  dplp::Promise<int> receiveIntP()
//  {
//      return receiveMessageP().then([](const std::string& message) {
//          std::size_t pos;
//          const int result = std::stoi(message, &pos);
//          if(pos != message.size())
//              throw std::runtime_error(
//                  "receiveIntP: trailing characters after int.");
//          return result;
//      });
//  }
//..
// This example brings up several interesting topics, the first of which is the
// result type of 'then', an int promise. 'then' infer an int promise result
// type because its argument (called the "fulfilled continuation"), which is
// itself a function, returns an int.
//
// The second notable point is what happens when the fulfilled continuation
// throws an exception. A runtime exception can occur if there are trailing
// characters after the 'int' and 'std::stoi' itself can throw several
// exceptions if there's invalid input. If any of these cases, the
// resulting int promise is rejected with precisely the exception that was
// thrown.
//
// Finally, it is important to note what happens if 'receiveMessageP''s result
// is rejected. When that is the case, the fulfilled continuation is never
// called and the result of 'receiveIntP' is rejected with the same exception
// that 'receiveMessageP''s result was rejected with.
//
///Example 4: Chaining promises
///- - - - - - - - - - - - - -
// 'then' doesn't always return a 'dplp::Promise<T>' when the fulfilled
// continuation returns a 'T'. When the fulfilled continuation returns a
// promise there is special behavior.
//
// Say we want to write a function that sends a message and then receives an
// int. Perhaps we are looking up named ints in some kind of database. Building
// off the previous examples, we could quickly come up with the following.
//..
//  dplp::Promise<int> lookupValue(const std::string& name)
//  {
//      return sendMessageP(name).then([](){
//          dplp::Promise<int> result = receiveIntP();
//          // What do we return?
//      });
//  }
//..
// Because sequencing promises one after another (chaining) is such a common
// operation, 'then' will automatically chain if the fulfilled continuation
// returns a promise. That is, if the fulfilled continuation returns
// 'dplp::Promise<T>', 'then' will return a 'dplp::Promise<T>' instead of a
// 'dplp::Promise<dplp::Promise<T>>'.
//
// To complete our example we simply return the result of 'receiveIntP'.
//..
//  dplp::Promise<int> lookupValue(const std::string& name)
//  {
//      return sendMessageP(name).then([](){
//          return receiveIntP();
//      });
//  }
//..
//
///Example 5: 'void'-returning continuations
///- - - - - - - - - - - - - - - - - - - - -
// Aside from promises, there are a couple other specially handled continuation
// return types. The first is the 'void' return type.
//..
//  dplp::Promise<> lookupAndPrintValue(const std::string& name)
//  {
//      return lookupValue(name).then([](int i){
//          std::cout << "Found value " << i << std::end;
//      });
//  }
//..
// Note that in the above example, the void return type of the fulfilled
// continuation produces an empty promise. 'dplp::Promise<>' serves the purpose
// of 'promise<void>' in other libraries.
//
// It is generally good practice to return a promise indicating when an
// operation has completed even if there is no value associated with this. This
// allows the caller to check for error conditions (described later) and chain
// further operations.
//
///Example 6: 'tuple'-returning continuations
///- - - - - - - - - - - - - - - - - - - - -
// A 'std::tuple' continuation return type is also specially handled. These
// produce promises that contain multiple values. Consider the following
// snippet which produces a 'dplp::Promise<std::string, int>'
//..
//  dplp::Promise<std::string, int> keyValue =
//    receiveMessageP().then([](const std::string& msg) {
//      return lookupValue(msg).then([msg](int value) {
//        return std::make_tuple(msg,value);
//      });
//    });
//..
// In this example, the result of 'receiveMessageP()' is sent to
// 'lookupValue()' and then the results of both are put into a tuple. The
// tuple, as a continuation, is becomes a 'dplp::Promise<std::string, int>'.
//
// Continuations for the 'then' method multi-valued promises take in multiple
// arguments. Here we create a new promise that outputs 'keyValue'.
//..
//  dplp::Promise<> keyValueOutputted =
//      keyValue.then([](const std::string& key, int value) {
//        std::cout << key << " -> " << value << std::endl;
//      });
//..
//
///Example 7: Handling Errors
///- - - - - - - - - - - - -
// As in life, one must account for broken (rejected) promises. The 'then'
// function's optional second argument is there for this purpose.
//..
//  dplp::Promise<> recieveAndPrintMessageAttempt = receiveMessageP()
//      .then(
//          [](const std::string& msg) {
//              std::cout << "Recieved Message: " << msg << std::endl;
//          },
//          [](const std::exception_ptr& error) {
//              std::cout << "There was an error, boo." << std::endl;
//          }
//      );
//..
// Similar to how 'then''s argument is called a fulfilled continuation,
// 'then''s second argument is called a rejected continuation. The fulfilled
// and rejected continuations must have the same return type.
//
// Note that because of the rejected continuation,
// 'recieveAndPrintMessageAttempt' is never rejected. Rejected continuations
// generally handle and recover from errors. However, if an unhandled exception
// is thrown from the rejected continuation, the resulting promise is rejected.
// The same goes for an unhandled exception thrown from the fulfilled
// continuation.
//
///Example 8: Looping via. Recursion
///- - - - - - - - - - - - - - - - -
// In asynchronous code, looping cannot be accomplished using the normal
// looping constructs such as 'for' or 'while'. Instead, recursion must be
// used.
//
// In the following example, an echo server will loop until the text "exit" is
// received.
//..
// dplp::Promise<> echoServer() {
//   receiveMessageP().then([](const std::string& msg) {
//      if(msg == "exit") {
//          return dplp::Promise<>([](auto fulfill, auto reject) {
//              fulfill();
//          });
//      } else {
//          return sendMessageP(msg).then(echoServer);
//      }
//   });
// }
//..
// The astute reader will raise a concerns about stack overflow which is
// normally a problem with recursive functions. Asynchronous recursion,
// however, works differently. The 'echoServer' function isn't actually called
// from within its body. Rather, one can think of it as being scheduled to
// execute again. In reality, there is no danger of stack overflow because the
// promise implementation doesn't keep track of "what to return to" as much as
// it keeps track of "what is the next operation to call".
//
///Example 9: Fulfill and reject helpers
///- - - - - - - - - - - - - - - - - - -
//
// One thing to note is that this expression,
//..
//  dplp::Promise<>([](auto fulfill, auto reject) { fulfill(); });
//..
// , in the above example is a common one. `dplp::Promise` has a `fulfill`
// static function that is a shorthand for this.
//..
//  dplp::Promise<>::fulfill();
//..
// This returns a fulfilled empty promise. You can use more arguments to
// 'fulfill' for creating non-empty promises.
//..
//  dplp::promise<int, std::string> foo
//    = dplp::Promise<>::fulfill(3, std::string("hello"));
//..
// Note that because 'fufill' deduces its return type from its arguments,
// '"string"' could not have been used here because a 'char*' would have been
// deduced as its type.
//
// Although not used much oustide of testing, the analog of 'fulfill'
// ('reject') is also provided. Unlike 'fulfill', the template arguments of
// 'promise' must be provided for the 'reject' function.

#include <dplmrts_anytuple.h>
#include <dplmrts_invocable.h>
#include <dplp_anypromise.h>
#include <dplp_promisestate.h>
#include <dplp_resolver.h>

#include <experimental/tuple>  // std::experimental::apply
#include <experimental/type_traits>  // std::experimental::is_void_v, std::experimental::is_same_v

#include <exception>    // std::exception_ptr
#include <functional>   // std::invoke
#include <tuple>        // std::tuple
#include <type_traits>  // std::result_of_t

namespace dplp {

template <typename T>
struct Promise_TupleContinuationThenResultImp {
};
template <typename... T>
struct Promise_TupleContinuationThenResultImp<std::tuple<T...> > {
    using type = dplp::Promise<T...>;
};
template <dplmrts::AnyTuple T>
using Promise_TupleContinuationThenResult =
    // 'Promise_TupleContinuationThenResult' is a type function that, when
    // given a tuple type, returns a promise type that has fufillment types
    // that match the element types of the tuple.
    typename Promise_TupleContinuationThenResultImp<T>::type;

template <typename... Types>
class Promise {
    // This class implements a value semantic type representing a heterogenius
    // sequence of values or exception at some point in time.
    //
    // Note that this is a lot like a 'future', but with different
    // constructors, simplified 'then' operations, and support for multiple
    // fulfilled values.

    // Promises may be copied. There are no semantic problems with this since
    // they have no mutating members. Well, at least until a 'cancel' operation
    // is implemented anyway.
    std::shared_ptr<dplp::PromiseState<Types...> > d_data_sp;

  public:
    Promise(dplp::Resolver<Types...> resolver);
        // Create a new 'Promise' object based on the specified 'resolver'.
        // 'resolver' is called exactly once by this constructor with a
        // 'dplmrts::Invocable<Types...>' (resolve function) as its first and a
        // 'dplmrts::Invocable<std::exception_ptr>' (reject function) as its
        // second argument.  When the resolve function is called, this promise
        // is fulfilled with its arguments. When the reject function is called,
        // this promise is rejected with its argument. The behavior is
        // undefined unless at most one of the arguments to 'resolver' is ever
        // called.
        //
        // Note that neither the reject nor resolve functions need be called
        // from within 'resolver'. 'resolver' could, for example, store these
        // functions elsewhere to be called at a later time.

    template <dplmrts::Invocable<Types...>           FulfilledCont,
              dplmrts::Invocable<std::exception_ptr> RejectedCont>
    Promise<>
    then(FulfilledCont fulfilledCont,
         RejectedCont  rejectedCont) const  // Two-argument version of case #1
        requires
        // void return type
        std::experimental::is_void_v<
            std::result_of_t<FulfilledCont(Types...)> >&&

        // 'fulfilledCont' and 'rejectedCont' have matching return values
        std::experimental::is_same_v<
            std::result_of_t<FulfilledCont(Types...)>,
            std::result_of_t<RejectedCont(std::exception_ptr)> >;
    template <dplmrts::Invocable<Types...> FulfilledCont>
    Promise<>
    then(FulfilledCont fulfilledCont) const  // One-argument version of case #1
        requires
        // void return type
        std::experimental::is_void_v<
            std::result_of_t<FulfilledCont(Types...)> >;
    template <dplmrts::Invocable<Types...>           FulfilledCont,
              dplmrts::Invocable<std::exception_ptr> RejectedCont>
    Promise_TupleContinuationThenResult<
        std::result_of_t<FulfilledCont(Types...)> >
    then(FulfilledCont fulfilledCont,
         RejectedCont  rejectedCont) const  // Two-argument version of case #2
        requires
        // tuple return type
        dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> >&&
        // 'fulfilledCont' and 'rejectedCont' have matching return values
        std::experimental::is_same_v<
            std::result_of_t<FulfilledCont(Types...)>,
            std::result_of_t<RejectedCont(std::exception_ptr)> >;
    template <dplmrts::Invocable<Types...> FulfilledCont>
    Promise_TupleContinuationThenResult<
        std::result_of_t<FulfilledCont(Types...)> >
    then(FulfilledCont fulfilledCont) const  // One-argument version of case #2
        requires
        // tuple return type
        dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> >;
    template <dplmrts::Invocable<Types...>           FulfilledCont,
              dplmrts::Invocable<std::exception_ptr> RejectedCont>
    std::result_of_t<FulfilledCont(Types...)>
    then(FulfilledCont fulfilledCont,
         RejectedCont  rejectedCont) const  // Two-argument version of case #3
        requires
        // promise return type
        dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> >&&
        // 'fulfilledCont' and 'rejectedCont' have matching return values
        std::experimental::is_same_v<
            std::result_of_t<FulfilledCont(Types...)>,
            std::result_of_t<RejectedCont(std::exception_ptr)> >;
    template <dplmrts::Invocable<Types...> FulfilledCont>
    std::result_of_t<FulfilledCont(Types...)>
    then(FulfilledCont fulfilledCont) const  // One-argument version of case #3
        requires
        // promise return type
        dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> >;
    template <dplmrts::Invocable<Types...>           FulfilledCont,
              dplmrts::Invocable<std::exception_ptr> RejectedCont>
        Promise<std::result_of_t<FulfilledCont(Types...)> >
        then(FulfilledCont fulfilledCont,
             RejectedCont  rejectedCont)
            const  // Two-argument version of case #4
        requires
        // non-void return type
        !std::experimental::is_void_v<
            std::result_of_t<FulfilledCont(Types...)> > &&
        // non-tuple return type
        !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> > &&
        // non-promise return type
        !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> > &&
        // 'fulfilledCont' and 'rejectedCont' have matching return values
        std::experimental::is_same_v<
            std::result_of_t<FulfilledCont(Types...)>,
            std::result_of_t<RejectedCont(std::exception_ptr)> >;
    template <dplmrts::Invocable<Types...> FulfilledCont>
        Promise<std::result_of_t<FulfilledCont(Types...)> >
        then(FulfilledCont fulfilledCont)
            const  // One-argument version of case #4
        requires
        // non-void return type
        !std::experimental::is_void_v<
            std::result_of_t<FulfilledCont(Types...)> > &&
        // non-tuple return type
        !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> > &&
        // non-promise return type
        !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> >;
        // Return a new promise that, upon the fulfilment of this promise, will
        // be fulfilled with the result of the specified 'fulfilledCont'
        // function or, upon reject of this promise, will be rejected with the
        // same 'std::exception_ptr'.
        //
        // Optionally specify a 'rejectedCont' function, which provides the
        // newly created promise with a fulfillment value in case this promise
        // is rejected. 'fulfilledCont' called with arguments of type
        // 'Types...' must have the same return type as 'rejectedCont' when
        // called with a 'std::exception_ptr'.
        //
        // If either 'fulfilledCont' or 'rejectedCont' throw an exception, the
        // resulting promise will be a rejected promise containing that
        // exception.
        //
        // The return type of 'fulfilledCont' controls the type of promise
        // returned from 'then'. There are the following cases:
        //
        // 1. If the return type of 'fulfilledCont' is 'void', the result of
        //    this function will be of type 'Promise<>'.
        // 2. If the return type of 'fulfilledCont' is 'tuple<T1,T2,...>' for
        //    some types 'T1,T2,...', the result of this function will be of
        //    type 'Promise<T1,T2,...>'.
        // 3. If the return type of 'fulfilledCont' is 'Promise<T1,T2,...>' for
        //    some types 'T1,T2,...', the result of this function will be of
        //    type 'Promise<T1,T2,...>'.
        // 4. If none of the above three conditions apply and the return type
        //    of 'fulfilledCont' is 'T', then the result of this function will
        //    be of type 'Promise<T>'.

    template <typename...                   Types2>
    static Promise<std::decay_t<Types2>...> fulfill(Types2&&... values);
        // Return a promise with the specified types that is fulfilled with the
        // specified 'values'.

    template <typename...     Types2>
    static Promise<Types2...> reject(std::exception_ptr error);
        // Return a promise with the specified types that is rejected with the
        // specified 'error'.

  private:
    Promise();
        // Create a new 'promise' object in the waiting state. It is never
        // fulfilled.
};

// ============================================================================
//                                 INLINE DEFINITIONS
// ============================================================================

template <typename... Types>
Promise<Types...>::Promise(dplp::Resolver<Types...> resolver)
: d_data_sp(std::make_shared<PromiseState<Types...> >())
{
    // Set 'fulfil' to the fulfilment function. Note that it, as well as
    // reject, keeps its own shared pointer to 'd_data_sp'.
    auto fulfil = [data_sp = this->d_data_sp](Types... fulfillValues) noexcept
    {
        data_sp->fulfill(std::move(fulfillValues)...);
    };

    auto reject = [data_sp = this->d_data_sp](std::exception_ptr e) noexcept
    {
        data_sp->reject(std::move(e));
    };

    std::invoke(resolver, std::move(fulfil), std::move(reject));
}

template <typename... Types>
template <dplmrts::Invocable<Types...>           FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
Promise<>                                        Promise<Types...>::then(
          FulfilledCont fulfilledCont,
          RejectedCont  rejectedCont) const  // Two-argument version of case #1
    requires
    // void return type
    std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)> >&&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)> >
{
    return Promise<>([
        this,
        fulfilledCont = std::move(fulfilledCont),
        rejectedCont  = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    std::invoke(std::move(fulfilledCont), t...);
                    fulfill();
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            [ rejectedCont = std::move(rejectedCont), fulfill, reject ](
                std::exception_ptr e) mutable {
                try {
                    std::invoke(std::move(rejectedCont), e);
                    fulfill();
                }
                catch (...) {
                    reject(std::current_exception());
                }
            });
    });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
Promise<>                              Promise<Types...>::then(
         FulfilledCont fulfilledCont) const  // One-argument version of case #1
    requires
    // void return type
    std::experimental::is_void_v<std::result_of_t<FulfilledCont(Types...)> >
{
    return Promise<>([ this, fulfilledCont = std::move(fulfilledCont) ](
        auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    std::invoke(std::move(fulfilledCont), t...);
                    fulfill();
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            reject);
    });
}

template <typename... Types>
template <dplmrts::Invocable<Types...>           FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
Promise_TupleContinuationThenResult<std::result_of_t<FulfilledCont(Types...)> >
Promise<Types...>::then(
          FulfilledCont fulfilledCont,
          RejectedCont  rejectedCont) const  // Two-argument version of case #2
    requires
    // tuple return type
    dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> >&&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)> >
{
    using Result = Promise_TupleContinuationThenResult<
        std::result_of_t<FulfilledCont(Types...)> >;

    return Result([
        this,
        fulfilledCont = std::move(fulfilledCont),
        rejectedCont  = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    std::experimental::apply(fulfill,
                                             std::move(fulfilledCont)(t...));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            [ rejectedCont = std::move(rejectedCont), fulfill, reject ](
                std::exception_ptr e) mutable {
                try {
                    std::experimental::apply(
                        fulfill, std::invoke(std::move(rejectedCont), e));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            });
    });
}

template <typename... Types>
template <dplmrts::Invocable<Types...> FulfilledCont>
Promise_TupleContinuationThenResult<std::result_of_t<FulfilledCont(Types...)> >
Promise<Types...>::then(
         FulfilledCont fulfilledCont) const  // One-argument version of case #2
    requires
    // tuple return type
    dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> >
{
    using Result = Promise_TupleContinuationThenResult<
        std::result_of_t<FulfilledCont(Types...)> >;

    return Result([ this, fulfilledCont = std::move(fulfilledCont) ](
        auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    std::experimental::apply(fulfill,
                                             std::move(fulfilledCont)(t...));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            reject);
    });
}

template <typename... Types>
template <dplmrts::Invocable<Types...>           FulfilledCont,
          dplmrts::Invocable<std::exception_ptr> RejectedCont>
std::result_of_t<FulfilledCont(Types...)>        Promise<Types...>::then(
    FulfilledCont fulfilledCont,
    RejectedCont  rejectedCont) const  // Two-argument version of case #3
    requires
    // promise return type
    dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> >&&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)> >
{
    using Result = std::result_of_t<FulfilledCont(Types...)>;

    return Result([
        this,
        fulfilledCont = std::move(fulfilledCont),
        rejectedCont  = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    Result innerPromise =
                        std::invoke(std::move(fulfilledCont), t...);
                    innerPromise.d_data_sp->postContinuations(fulfill, reject);
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            [ rejectedCont = std::move(rejectedCont), fulfill, reject ](
                std::exception_ptr e) mutable {
                try {
                    Result innerPromise =
                        std::invoke(std::move(rejectedCont), e);
                    innerPromise.d_data_sp->postContinuations(fulfill, reject);
                }
                catch (...) {
                    reject(std::current_exception());
                }
            });
    });
}

template <typename... Types>
template <dplmrts::Invocable<Types...>    FulfilledCont>
std::result_of_t<FulfilledCont(Types...)> Promise<Types...>::then(
    FulfilledCont fulfilledCont) const  // One-argument version of case #3
    requires
    // promise return type
    dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> >
{
    using Result = std::result_of_t<FulfilledCont(Types...)>;

    return Result([ this, fulfilledCont = std::move(fulfilledCont) ](
        auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    Result innerPromise =
                        std::invoke(std::move(fulfilledCont), t...);
                    innerPromise.d_data_sp->postContinuations(fulfill, reject);
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            reject);
    });
}

template <typename... Types>
    template <dplmrts::Invocable<Types...>           FulfilledCont,
              dplmrts::Invocable<std::exception_ptr> RejectedCont>
    Promise<std::result_of_t<FulfilledCont(Types...)> >
    Promise<Types...>::then(
          FulfilledCont fulfilledCont,
          RejectedCont  rejectedCont) const  // Two-argument version of case #4
    requires
    // non-void return type
    !std::experimental::is_void_v<
        std::result_of_t<FulfilledCont(Types...)> > &&

    // non-tuple return type
    !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> > &&

    // non-promise return type
    !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> > &&

    // 'fulfilledCont' and 'rejectedCont' have matching return values
    std::experimental::is_same_v<
        std::result_of_t<FulfilledCont(Types...)>,
        std::result_of_t<RejectedCont(std::exception_ptr)> >
{
    using U = std::result_of_t<FulfilledCont(Types...)>;

    return Promise<U>([
        this,
        fulfilledCont = std::move(fulfilledCont),
        rejectedCont  = std::move(rejectedCont)
    ](auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    fulfill(std::invoke(std::move(fulfilledCont), t...));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            [ rejectedCont = std::move(rejectedCont), fulfill, reject ](
                std::exception_ptr e) mutable {
                try {
                    fulfill(std::invoke(std::move(rejectedCont), e));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            });
    });
}

template <typename... Types>
    template <dplmrts::Invocable<Types...> FulfilledCont>
    Promise<std::result_of_t<FulfilledCont(Types...)> >
    Promise<Types...>::then(
         FulfilledCont fulfilledCont) const  // One-argument version of case #4
    requires
    // non-void return type
    !std::experimental::is_void_v<
        std::result_of_t<FulfilledCont(Types...)> > &&

    // non-tuple return type
    !dplmrts::AnyTuple<std::result_of_t<FulfilledCont(Types...)> > &&

    // non-promise return type
    !dplp::AnyPromise<std::result_of_t<FulfilledCont(Types...)> >
{
    using U = std::result_of_t<FulfilledCont(Types...)>;

    return Promise<U>([ this, fulfilledCont = std::move(fulfilledCont) ](
        auto fulfill, auto reject) mutable {
        d_data_sp->postContinuations(
            [ fulfilledCont = std::move(fulfilledCont), fulfill, reject ](
                Types... t) mutable {
                try {
                    fulfill(std::invoke(std::move(fulfilledCont), t...));
                }
                catch (...) {
                    reject(std::current_exception());
                }
            },
            [reject](std::exception_ptr e) mutable { reject(e); });
    });
}

template <typename... Types>
template <typename...            Types2>
Promise<std::decay_t<Types2>...> Promise<Types...>::fulfill(Types2&&... values)
{
    Promise<std::decay_t<Types2>...> result;
    result.d_data_sp->fulfill(std::forward<Types2>(values)...);
    return result;
}

template <typename... Types>
template <typename... Types2>
Promise<Types2...> Promise<Types...>::reject(std::exception_ptr error)
{
    Promise<Types2...> result;
    result.d_data_sp->reject(std::move(error));
    return result;
}

template <typename... Types>
Promise<Types...>::Promise()
: d_data_sp(std::make_shared<PromiseState<Types...> >())
{
}
}

#endif
