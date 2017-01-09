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

## Building

This group can be built with GCC 6.2, CMake, and a recent version of `gtest`.
To get such an environment on a Bloomberg Linux machine, such as apinjdev02,
add something like the following to your `~/.bash_profile`,

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

, and then build with CMake using something like the following commands:

```bash
mkdir build
cd build
CXX=g++-6.2 cmake /path/to/source
make -j4
```

. The unit tests can be built and invoked using 'ctest'.
