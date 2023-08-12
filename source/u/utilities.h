// Copyright (C) 2023 King E. Lanchester
// SPDX-License-Identifier: MIT

#pragma once
#define U_INCLUDED_UTILITIES_H

namespace u
{

template<typename ...Ts>
constexpr void discard(Ts...) noexcept {}

}
