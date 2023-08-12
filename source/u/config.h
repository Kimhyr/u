#pragma once
#define U_INCLUDED_CONFIG_H

#if __cplusplus < 202002L
#	error C++ version is too old (must be C++20 or newer)
#endif

#if !defined __clang__ || !defined __GNUC__
#	error unknown compiler (use Clang or the GCC)
#endif

#if !defined __x86_64__
#	error invalid architecture (compile for x86-64)
#endif

#define U_ENABLE_UNPREFIXED_MACROS

#if !defined U_THROW
#if __cpp_exceptions
#	define U_THROW(v) (throw (v))
#else
#	defined U_THROW(v) (__builtin_abort())
#endif
#endif

#if defined U_ENABLE_UNPREFIXED_MACROS
#	define THROW U_THROW
#endif
