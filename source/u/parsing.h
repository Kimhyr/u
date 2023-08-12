// Copyright (C) 2023 King E. Lanchester
// SPDX-License-Identifier: MIT

#pragma once
#define U_INCLUDED_PARSING_H

#include <u/diagnostics/result.h>

namespace u
{

template<typename T, typename E>
u::result<T, E> parse() noexcept;


}
