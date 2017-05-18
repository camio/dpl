# `dplp`

**PURPOSE:** Provide a promise vocabulary type for asynchronous applications.

**MNEMONIC:** David's primitive library (dpl) promises (p)

## Description

The `dplp` package provides components that comprise a vocabulary type,
`dplp::Promise`, which can be used to develop asynchronous applications in a
highly composable way. The `dplp_promise` component contains the core of this
functionality.

Promises are intended to be used as the result of asynchronous operations and
can be chained together in various ways. It is modeled after the JavaScript
promises standard.

What follows is some example promise code. In this scenario the code connects
to a server to get a request, handles it by possibly connecting asynchronously
to a database, and sends the response back to the server.

```c++
server myServer;

dplp::Promise<Request> requestPromise = myServer.getPendingRequest(/*...*/);

requestPromise
.then( [](Request request){
  if(request.wantsDatabaseName()) {
    return lookupInDatabase(/*...*/); // 'lookupInDatabase' returns
                                      // 'Promise<std::string>'
  } else {
    assert(request.wantsHardcodedName());
    // 'fulfill' also returns a 'Promise<std::string>'.
    return dplp::Promise<>::fulfill(std::string("Joe"));
  }
})
.then( [myServer&](std::string s) {
  return myServer.sendResponse(s); // 'sendResponse' returns a
                                   // 'Promise<>'.
})
// Here we use the two-argument version of 'then'.
.then( [myServer&]{
  // If everything is good, close the connection.
  myServer.closeConnection();
}, [](std::exception_ptr rejectedValue) {
  // If anything goes wrong, log it and close the connection.
  try
  {
    std::rethrow_exception(rejectedValue);
  }
  catch( std::exception & e )
  {
    std::cout << "Something bad happened: " << e.what() << std::endl;
  }
  myServer.closeConnection();
});

// Blocking call to begin the event loop until completion.
myServer.run();
```

## Hierarchical Synopsis

The `dplp` package currently has 6 components having 5 levels of physical
dependency.

```
5. dplp_promise

4. dplp_promisestate

3. dplp_promisestateimputil

2. dplp_promisestateimp

1. dplp_anypromise
   dplp_resolver
```

## Component Synopsis

* `dplp_anypromise`.
    Provide a concept that is satisfied by promise types.
* `dplp_promise`.
    Provide a template representing asynchronous values.
* `dplp_promisestate`.
    Provide a low-level promise with only fundamental operations.
* `dplp_promisestateimp`.
    Provide datatypes for representing promise state.
* `dplp_promisestateimputil`.
    Provide utility functions for 'dplp::PromiseStateImp' objects.
* `dplp_resolver`.
    Provide a concept that is satisfied by promise resolver functions.

## License

Most of this code is licensed under the Apache License, but there is third
party code also incorporated that has its own license. See the dplm17 and
dplm20 packages for more details.

## TODO

### Priority 2
- Add unit tests to verify that callables with move-only call operators are
  supported as continuation functions and resolvers.
- clean up the test cases now that we have 'fulfilled' and 'rejected' static
  functions.
- Figure out how to 'std::forward' into a lambda and use it in the 'then'
  functions.
- Make the 'fulfil' and 'reject' functions which are passed to the 'resolver'
  in the 'Promise' constructor release their reference to 'd_data' when one of
  the two is called. The only way that I can think of doing this is to make a
  `shared_ptr<shared_ptr<data>>` object that both of those functions reference.
  At the first call, the "inner" shared pointer is made null. Unit tests
  should be created for this.
- Carefully go over unit tests to be sure all branches are being tested.

### Priority 3
- Add unit tests that verify that exceptions in the resolver are rethrown.
- Add unit tests that verify that rejection continuations are called properly
  and that if they throw, it is appropriately handled.
- Add a unit tests that verifies that `promise<std::exception_ptr>` works
  properly.
- Add support for a then result of a 'keep' which keeps the result value no
  matter what its type is.
- Add cancellation support
- In the context of 'then', if 'ef's return value is convertable to 'f's return
  value, that should be okay.
- Consider relaxing "'fulfilledCont' called with arguments of type 'Types...'
  must have the same return type as 'rejectedCont' when called with a
  'std::exception_ptr'" so that these types only need to have a
  'std::common_type' type. We may need a new 'std::has_common_type' for the
  concept for this.
- Consider adding a second 'rejected' function which uses the Types argument of
  the promise. Really, these things should be independent of the class template
  in a PromiseUtil.
- Consider how one would write a 'fulfil' static function with move semantics
  that doesn't use any private members.
- Add an optimization that assumes only a single instance of the promise. This
  should allow for inlining and allocation avoidance. There would be an
  internal state that would switch between a shared_ptr data and an internal
  data.

## Problems with `std::future` and `std::promise`
- Pain to get exceptions out.
- No "then" hampers usage.
- Don't want blocking '.get'
- "void" used instead of <>
- shared_future: comprimized because they didn't use value semantics.
- individual threads cannot share a shared_future instance.
- valid vs. nonvalid state. Confusing.
- `std::async` sucks. Weird.
- tied to thread (see destructor)
- `set_value_at_thread_exit` promises.
- No short circuiting when an exception is thrown at the beginning of a long
  continuation chain.
- When any works much better with a different model, my model in particular. It
  should return a future variant.
- Very hard to teach. Why do we need to move this? Why do we have two? etc.
- Synchronization points are made explicit with a promise based API. Do x after
  I know I won't be recieving any new requests.

## References

- [Concepts TS (N4361)](http://open-std.org/JTC1/SC22/WG21/docs/papers/2015/n4361.pdf)
- [Ranges TS (N4569)](http://open-std.org/JTC1/SC22/WG21/docs/papers/2016/n4569.pdf)
- [Google Closure Promises](https://github.com/google/closure-library/blob/master/closure/goog/promise/promise.js)
- [Promises/A+ JavaScript Promise Standard](http://open-std.org/JTC1/SC22/WG21/docs/papers/2016/n4569.pdf)
