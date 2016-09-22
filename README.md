# Promise Library

TODO:
  - Use invokable teminology instead of callable.


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
