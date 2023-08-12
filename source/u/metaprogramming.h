// Copyright (C) 2023 King E. Lanchester
// SPDX-License-Identifier: MIT

#pragma once
#define U_INCLUDED_METAPROGRAMMING_H

#include <concepts>
#include <type_traits>

namespace u
{

template<typename ClassType, typename... ParameterTypes>
struct is_explicity_constructible
    : std::bool_constant<
        std::is_constructible_v<ClassType, ParameterTypes...>
        && !std::is_convertible_v<ClassType, ParameterTypes...>>
{};

template<typename T, typename... Ts>
constexpr bool is_explicity_constructible_v = is_explicity_constructible<T, Ts...>::value;

template<typename T>
struct is_cv
    : std::bool_constant<
        std::is_const_v<T>
        && std::is_volatile_v<T>>
{};

template<typename T>
constexpr bool is_cv_v = is_cv<T>::value;

}
