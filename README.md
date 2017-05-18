# `dpl`

**PURPOSE:** Provide foundational components for modern development

**MNEMONIC:** David's primitive library (dpl)

## Description

The `dpl` ("David's Primitive Library") package group contains several
general purpose packages that are dependent only on C++14 and libraries that
have been released in the form of a technical specification (TS).

## Hierarchical Synopsis

The `dpl` package group currently has 4 packages having 2 levels of physical
dependency. The list below shows the hierarchical ordering of the packages.

```
2. dplp

1. dplm17
   dplm20
   dplmrts
```

## Package Overview

This section is a brief introduction to the packages of the `dpl` package
group.  See the respective Package Level documents for more details.

### `dplm17`

The `dplm17` package provides libraries that are expected to be included in the
upcoming C++17 standard, but are not yet widely distributed with standard
library implementations. Currently, the only component included in this package
is `dplm17_variant`.

### `dplm20`

The `dplm20` package provides libraries that are expected to be included in the
upcoming C++20 standard, but are not yet widely distributed with standard
library implementations. Currently, the only component included in this package
is `dplm20_overload`.

### `dplmrts`

The `dplmrts` package provides components that are expected to be included now,
or eventually, in the Ranges TS. The Ranges TS includes components that make
good use of the Concepts TS language feature. This package provides only a
subset of the functionality in the Ranges TS.

### `dplp`

The `dplp` package provides components that comprise a vocabulary type,
`dplp::Promise`, which can be used to develop asynchronous applications in a
highly composable way. The `dplp_promise` component contains the core of this
functionality.
