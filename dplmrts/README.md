# `dplmrts`

**PURPOSE:** Provide library components based on the Concepts TS.

**MNEMONIC:** David's primitive library (dpl) Missing Ranges TS (mrts)

**AUTHOR:** David Sankel (dsankel)

## Description

The `dplmrts` package provides components that are expected to be included now,
or eventually, in the Ranges TS. The Ranges TS includes components that make
good use of the Concepts TS language feature. This package provides only a
subset of the functionality in the Ranges TS.

## Hierarchical Synopsis

The `dplmrts` package currently has 3 components having 2 levels of physical
dependency.

```
2. dplmrts_invocablearchetype

1. dplmrts_anytuple
   dplmrts_invocable
```

## Component Synopsis

* `dplmrts_anytuple`. Provide a concept that is satisfied by tuple types.
* `dplmrts_invocable`. Provide a concept that is satisfied by invocable types.
* `dplmrts_invocablearchetype`.
