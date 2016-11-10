#ifndef DPLMRTS_INVOCABLEARCHETYPE_INCLUDED
#define DPLMRTS_INVOCABLEARCHETYPE_INCLUDED

#include <dplmrts_invocable.h>

namespace dplmrts {
// 'InvocableArchetype<Types...>' is a type that satisifies
// 'dplmrts::Invocable<Types...>'.
template <typename... Types> class InvocableArchetype {
  void operator()(Types...) const {}
};
}

#endif
