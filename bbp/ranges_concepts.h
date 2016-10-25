#ifndef BBP_RANGES_CONCEPTS_INCLUDED
#define BBP_RANGES_CONCEPTS_INCLUDED

// PURPOSE: Provide some concepts from the Ranges TS

namespace bbp {

// This is an abridged implementation of the like named 'Callable' concept in
// the Range's TS.
template <typename F, typename... Types>
concept bool Callable = requires(F f, Types... t)
{
    f(t...);
};
}

#endif
