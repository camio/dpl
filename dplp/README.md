# Promise Library

**PURPOSE:** Provide a promise vocabulary type for asynchronous applications.

Files:

- **dplp_promise.h**. Contains the `dplp::promise` class.
- **dplp_promise.t.cpp**. Test driver for the above component. Uses
  `gtest`.

Promises are intended to be used as the result of asynchronous operations and
can be chained together in various ways. It is modeled after the JavaScript
promises standard.

What follows is some example promise code. In this scenario the code connects
to a server to get a request, handles it by possibly connecting asynchronously
to a database, and sends the response back to the server.

```c++
server myServer;

dplp::promise<Request> requestPromise = myServer.getPendingRequest(/*...*/);

requestPromise
.then( [](Request request){
  if(request.wantsDatabaseName()) {
    return lookupInDatabase(/*...*/); // 'lookupInDatabase' returns
                                      // 'promise<std::string>'
  } else {
    assert(request.wantsHardcodedName());
    // 'fulfill' also returns a 'promise<std::string>'.
    return dplp::promise<>::fulfill(std::string("Joe"));
  }
})
.then( [myServer&](std::string s) {
  return myServer.sendResponse(s); // 'sendResponse' returns a
                                   // 'promise<>'.
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

## Building

This can be built with GCC 6.2 and a recent version of `gtest`. To get such an environment on a Bloomberg Linux machine, such as apinjdev02, add something like the following to your `~/.bash_profile`,

```bash
# GCC
export PATH=/home/dsankel/bs/opt/gcc/bin:$PATH
export LD_LIBRARY_PATH=/home/dsankel/bs/opt/gcc/lib64:/home/dsankel/bs/opt/gccdeps/lib:$LD_LIBRARY_PATH
export MANPATH=/home/dsankel/bs/opt/gcc/share/man:$MANPATH
export INFOPATH=/home/dsankel/bs/opt/gcc/share/info:$INFOPATH

# Google Test
export LIBRARY_PATH=/home/dsankel/bs/opt/gtest/lib:$LIBRARY_PATH
export CPATH=/home/dsankel/bs/opt/gtest/include:$CPATH
export PKG_CONFIG_PATH=/home/dsankel/bs/opt/gtest/lib/pkgconfig:$PKG_CONFIG_PATH
```

, and then build with the following command,

```bash
CXX=g++-6.2 make
```

. This will build and run the unit tests.

## TODO's

### Priority 1
- Make the 'then' function and the 'resolve' and 'reject' functions generated
  in the constructor thread safe. I think, in essence, d_data and all it points
  to needs to be protected with a mutex.
- Make all the comments follow the signatures in BDE style.
- Look at other ways to make this more closely follow the BDE style.
- Consider how to properly handle cv qualifiers in continuation return types.

### Priority 2

- Add unit tests to verify that callables with move-only call operators are
  supported as continuation functions and resolvers.
- clean up the test cases now that we have 'fulfilled' and 'rejected' static
  functions.
- Consider whether or not all function arguments should be passed by && and
  'std::forward' should be used.
- Consider what happens when the 'fufil' and 'reject' functions passed to the
  resolver throw an exception. We need to properly support this.
- Make the 'fulfil' and 'reject' functions which are passed to the 'resolver'
  in the 'promise' constructor release their reference to 'd_data' when one of
  the two is called. The only way that I can think of doing this is to make a
  `shared_ptr<shared_ptr<data>>` object that both of those functions reference.
  At the first call, the "inner" shared pointer is made null. Unit tests
  should be created for this.
- Make the 'then' functions 'const' since they don't change the semantic value
  of a promise.
- Carefully go over unit tests to be sure all branches are being tested.

### Priority 3
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
