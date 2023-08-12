// Copyright (C) 2023 King E. Lanchester
// SPDX-License-Identifier: MIT

#pragma once
#define U_INCLUDED_DIAGNOSTICS_ASSERTING_H

#include <type_traits>

namespace u
{

namespace detail
{

template<typename T, T A, typename U, U B>
constexpr void assert_equal(std::integral_constant<T, A>,
                            std::integral_constant<U, B>)
{ static_assert(A == B, "equavalent assertion failed"); }

}  // namespace detail

#define U_ASSERT_EQUAL(a, b) \
    assert_equal(constant<decltype(a), a>{}, constant<decltype(b), b>{})

#if !defined U_PREFIX_MACROS
#   define ASSERT_EQUAL(a, b) U_ASSERT_EQUAL
#endif

}
