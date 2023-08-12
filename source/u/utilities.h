// Copyright (C) 2023 King E. Lanchester
// SPDX-License-Identifier: MIT

#pragma once
#define U_INCLUDED_UTILITIES_H

namespace core
{

template<typename ...Ts>
constexpr void discard(Ts...) noexcept {}

}
