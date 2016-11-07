# dpl_m17

**MNEMONIC:** David's primitive library (DPL) Missing C++ 17 (m17)

**PURPOSE:** Provide some C++17 library components.

- **dpl/bbp/variant.h**. An implementation of the standard `<variant>` header.
  This differs from the specification in that the namespace used is `dpl::m17`
  instead of `std`.  This is derived from a copy of Anthony Williams's
  'std::variant' implementation. See the code for copywrite information.
