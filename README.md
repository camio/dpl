# Promise Library

This library makes use of a variant library in the 'bbp' namespace. This is a
copy of Anthony Williams 'std::variant' implementation.

## TODO's

- Add unit tests to verify that move-only continuation functions work properly.
- Think about adding support for "call-once" functions (viz. those that have
  only a && call operator defined.) in both continuations and resolvers.
- Add support for a then result of a promise producing a promise of the
  internal type instead of a promise promise.
- clean up the test cases now that we have 'fulfilled' and 'rejected' static
  functions.
- Add cancelation support
- Add support for a then result of a tuple.
- Add support for a then result of a 'keep' which keeps the result value no
  matter what its type is.
- All function arguments should be by &&
- In the context of 'then', if 'ef's return value is convertable to 'f's return
  value, that should be okay.
- For this implementation to work properly, we are guarenteeing that the fulfil
  and reject functions passed to the resolver do not throw exceptions. To fix
  this, I need to ensure that my "try catch" block is over the call of the
  functions passed into then. The result then needs to be captured and used
  later.
- The common-type of the results of the 'f' and 'ef' functions should be used
  as the resulting promise's type  for the two arguent versions of 'then'.
- Consider adding a second 'rejected' function which uses the Types argument of
  the promise. Really, these things should be independent of the class template
  in a PromiseUtil.
- Move these TODO's elsewhere.
- Consider how one would write a 'fulfilled' with move semantics that doesn't
  use any private members.
- Add an optimization that assumes only a single instance of the promise. This
  should allow for inlining and allocation avoidance. There would be an
  internal state that would switch between a shared_ptr data and an internal
  data.
- Make these things thread safe. Accesses to the stuff pointed to by 'd_data'
  need to be protected by a mutex I think.
- Consider a type other than the vector of function pairs for the waiting state
  data. I don't like the idea that it may allocate twice for this.
- Make the 'fulfil' and 'reject' functions which are passed to the 'resolver'
  in the 'promise' constructor release their reference to 'd_data' when one of
  the two is called. The only way that I can think of doing this is to make a
  `shared_ptr<shared_ptr<data>>` object that both of those functions reference.
  At the first call, the "inner" shared pointer is made null.
- Make all the comments follow the signatures.


## Concepts TS
N4361 http://open-std.org/JTC1/SC22/WG21/docs/papers/2015/n4361.pdf

Ugh. It's easy to make a concept for a callable, but difficult to make a
concept for a callable that takes a callable argument.

## Ranges TS
N4569 http://open-std.org/JTC1/SC22/WG21/docs/papers/2016/n4569.pdf

## Closure Promises
https://github.com/google/closure-library/blob/master/closure/goog/promise/promise.js

## Notes
- Synchronization points are made explicit with a promise based API. Do x after
  I know I won't be recieving any new requests.

## Problems with Futures
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
