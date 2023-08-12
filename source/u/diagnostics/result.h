// Copyright (C) 2023 King E. Lanchester
// SPDX-License-Identifier: MIT

#pragma once
#define U_INCLUDED_DIAGNOSTICS_RESULT_H

#include <u/config.h>

#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <u/metaprogramming.h>

namespace u
{

struct err_t {};

template<typename ErrorType>
class error;

template<typename ErrorType>
class bad_result_access;

template<typename T>
struct is_error
	: std::bool_constant<false>
{};

template<typename T>
struct is_error<u::error<T>>
	: std::bool_constant<true>
{};

template<typename T>
constexpr bool is_error_v = u::is_error<T>::value;

template<typename T>
struct is_valid_result
	: std::bool_constant<
		!u::is_error_v<std::remove_cvref_t<T>>
		&& !std::is_reference_v<T>
		&& !std::is_function_v<T>>
{};

template<typename T>
constexpr bool is_valid_result_v = u::is_valid_result<T>::value;

template<typename T>
struct is_valid_error
	: std::bool_constant<
		!u::is_error_v<std::remove_cvref_t<T>>
		&& !u::is_cv_v<T>
		&& !std::is_array_v<T>
		&& std::is_object_v<T>>
{};

template<typename T>
constexpr bool is_valid_error_v = u::is_valid_error<T>::value;

template<>
class bad_result_access<void>
	: public std::exception
{
public:
	[[nodiscard]]
	const char* what() const noexcept override
	{ return "bad_result_access"; }

protected:
	bad_result_access() noexcept {}
	bad_result_access(const bad_result_access&) = default;
	bad_result_access(bad_result_access&&) = default;

	bad_result_access& operator=(const bad_result_access&) = default;
	bad_result_access& operator=(bad_result_access&&) = default;

	~bad_result_access() = default;
};

template<typename ErrorType>
class bad_result_access
	: public bad_result_access<void>
{
	static_assert(u::is_valid_error_v<ErrorType>);
		
public:
	explicit bad_result_access(ErrorType error)
		: m_error{error} {}

	[[nodiscard]]
	ErrorType& error() & noexcept
	{ return this->m_error; }

	[[nodiscard]]
	ErrorType&& error() && noexcept
	{ return std::move(this->m_error); }

	[[nodiscard]]
	const ErrorType&& error() const&& noexcept
	{ return std::move(this->m_error); }

private:
	ErrorType m_error;
};

namespace detail::result_helpers
{

template<typename Gaurded>
	requires std::is_nothrow_move_constructible_v<Gaurded>
struct guard
{
	constexpr guard(Gaurded& target) noexcept
		: m_target{std::addressof(target)},
		  m_temp{std::move(target)}
	{ std::destroy_at(this->m_target); }

	constexpr ~guard() noexcept
	{
		if (this->m_target) [[unlikely]]
			std::construct_at(
				this->target,
				std::move(this->temp));
	}

	guard(const guard&) = delete;
	guard& operator=(const guard&) = delete;

	constexpr Gaurded&& release() noexcept
	{
		this->m_target = nullptr;
		return std::move(this->m_temp);
	}

private:
	Gaurded* m_target;
	Gaurded m_temp;
};

template<typename New, typename Old, typename Arg>
constexpr void reconstruct(New* new_, Old* old, Arg&& arg)
noexcept(std::is_nothrow_constructible_v<New, Arg>)
{
	if constexpr(std::is_nothrow_constructible_v<New, Arg>) {
		std::destroy_at(old);
		std::construct_at(new_, std::forward<Arg>(arg));
		return;
	} else if constexpr(std::is_nothrow_move_constructible_v<Arg>) {
		New temp{std::forward<Arg>(arg)};
		std::destroy_at(old);
		std::construct_at(new_, std::move(temp));
		return;
	}

	result_helpers::guard<Old> guard{*old};
	std::construct_at(new_, std::forward<Arg>(arg));
	guard.target = nullptr;
}

}  // namespace detail::result_helpers

template<typename ValueType, typename ErrorType>
class result
{
	static_assert(u::is_valid_result_v<ValueType>);
	static_assert(u::is_valid_error_v<ErrorType>);

public:
	template<template<typename...> typename T>
	static constexpr bool conjunction_v =
		T<ValueType>::value
		&& T<ErrorType>::value;

	template<template<typename...> typename T, typename U, typename V>
	static constexpr bool conjunction_with_v =
		T<ValueType, U>::value
		&& T<ErrorType, V>::value;

	template<template<typename...> typename T, typename U, typename V>
	static constexpr bool conjunction_with_before_v =
		T<U, ValueType>::value
		&& T<V, ErrorType>::value;

	template<template<typename...> typename T>
	static constexpr bool disjunction_v =
		T<ValueType>::value
		|| T<ErrorType>::value;

	template<template<typename...> typename T, typename U, typename V>
	static constexpr bool disjunction_with_v =
		T<ValueType, U>::value
		|| T<ErrorType, V>::value;

	template<template<typename...> typename T, typename U, typename V>
	static constexpr bool disjunction_with_before_v =
		T<U, ValueType>::value
		|| T<V, ErrorType>::value;

private:
	template<typename T, typename U>
	static constexpr bool m_is_constructible_from_result_v =
		conjunction_with_v<std::is_constructible, T, U>
		&& !(std::is_constructible_v<ValueType, result<T, U>>
			|| std::is_constructible_v<ValueType, result<T, U>&>
			|| std::is_constructible_v<ValueType, const result<T, U>>
			|| std::is_constructible_v<ValueType, const result<T, U>&>
			|| std::is_constructible_v<result<T, U>, ValueType>
			|| std::is_constructible_v<result<T, U>&, ValueType>
			|| std::is_constructible_v<const result<T, U>, ValueType>
			|| std::is_constructible_v<const result<T, U>, ValueType>
			|| std::is_constructible_v<error<ErrorType>, result<T, U>>
			|| std::is_constructible_v<error<ErrorType>, result<T, U>&>
			|| std::is_constructible_v<error<ErrorType>, const result<T, U>>
			|| std::is_constructible_v<error<ErrorType>, const result<T, U>&>);

	template<typename T, typename U>
	static constexpr bool m_is_explicitly_constructible_from_result_v =
		disjunction_with_before_v<std::is_convertible, T, U>;

	template<typename T, typename U>
	static constexpr bool m_is_nothrow_constructible_from_result_v = 
		conjunction_with_v<std::is_nothrow_constructible, T, U>;

	template<typename T, typename U, typename... Ts>
	static constexpr bool m_is_constructible_with_il_v =
		std::is_constructible_v<T, std::initializer_list<U>&, Ts...>;

	template<typename T, typename U, typename... Ts>
	static constexpr bool m_is_nothrow_constructible_with_il_v =
		std::is_nothrow_constructible_v<T, std::initializer_list<U>&, Ts...>;

	static constexpr bool m_is_copy_or_move_assignable_v =
		conjunction_v<std::is_copy_assignable>
		&& conjunction_v<std::is_copy_constructible>
		&& disjunction_v<std::is_nothrow_move_constructible>;

	static constexpr bool m_is_nothrow_copy_or_move_assignable_v =
		conjunction_v<std::is_nothrow_copy_constructible>
		&& conjunction_v<std::is_copy_assignable>;

	template<typename T>
	static constexpr bool m_is_assignable_with_error_v =
		std::is_constructible_v<ErrorType, const T&>
		&& std::is_assignable_v<ErrorType&, const T&>
		&& (std::is_nothrow_constructible_v<ErrorType, const T&>
			|| disjunction_v<std::is_nothrow_move_constructible>);

public:
	constexpr result()
	noexcept(std::is_nothrow_default_constructible_v<ValueType>)
		requires std::is_default_constructible_v<ValueType>
		: m_value{}
	{}

	result(const result&) = default;

	constexpr result(const result& other)
	noexcept(conjunction_v<std::is_nothrow_copy_constructible>)
		requires conjunction_v<std::is_copy_constructible>
			&& (!disjunction_v<std::is_trivially_copy_constructible>)
		: m_has_value{other.m_has_value}
	{
		if (this->m_has_value)
			std::construct_at(
				std::addressof(this->m_value),
				other.m_value);
		else std::construct_at(
			std::addressof(this->m_error),
			other.m_error);
	}

	result(const result&&) = default;

	constexpr result(result&& other)
	noexcept(conjunction_v<std::is_nothrow_move_constructible>)
		requires conjunction_v<std::is_move_constructible>
			&& (!disjunction_v<std::is_trivially_move_constructible>)
		: m_has_value{other.m_has_value}
	{
		if (this->m_has_value)
			std::construct_at(
				std::addressof(this->m_value),
				std::move(other.m_value));
		else std::construct_at(
			std::addressof(this->m_error),
			std::move(other.m_error));
	}

	template<typename T, typename U>
		requires m_is_constructible_from_result_v<T, U>
	constexpr explicit(m_is_explicitly_constructible_from_result_v<T, U>)
	result(const result<T, U>& other)
	noexcept(m_is_nothrow_constructible_from_result_v<T, U>)
		: m_has_value{other.m_has_value}
	{
		if (this->m_has_value)
			std::construct_at(
				std::addressof(this->m_value),
				other.m_value);
		else std::construct_at(
			std::addressof(this->m_error),
			other.m_error);
	}

	template<typename T, typename U>
		requires m_is_constructible_from_result_v<T, U>
	constexpr explicit(m_is_explicitly_constructible_from_result_v<T, U>)
	result(result<T, U>&& other)
	noexcept(m_is_nothrow_constructible_from_result_v<T, U>)
		: m_has_value{other.m_has_value}
	{
		if (this->m_has_value)
			std::construct_at(
				std::addressof(this->m_value),
				std::move(other.m_value));
		else std::construct_at(
			std::addressof(this->m_error),
			std::move(other.m_error));
	}

	template<typename T = ValueType>
		requires (!std::is_same_v<std::remove_cvref_t<T>, std::in_place_t>)
			&& (!std::is_same_v<std::remove_cvref_t<T>, result>)
			&& std::is_constructible_v<ValueType, T>
			&& (!is_error_v<std::remove_cvref_t<T>>)
	constexpr explicit(!std::is_convertible_v<T, ValueType>)
	result(T&& value)
	noexcept(std::is_nothrow_constructible_v<ValueType, T>)
		: m_value{std::forward<T>(value)}
	{}

	template<typename T = ErrorType>
		requires std::is_constructible_v<ErrorType, const T&>
	constexpr explicit(!std::is_convertible_v<const T&, ErrorType>)
	result(const error<T>& error)
	noexcept(std::is_nothrow_constructible_v<ErrorType, const T&>)
		: m_error{error.error()},
		  m_has_value{false}
	{}

	template<typename T = ErrorType>
		requires std::is_constructible_v<ErrorType, T>
	constexpr explicit(!std::is_convertible_v<T, ErrorType>)
	result(error<T>&& error)
	noexcept(std::is_nothrow_constructible_v<ErrorType, T>)
		: m_error{std::move(error).error()},
		  m_has_value{false}
	{}

	constexpr explicit result(std::in_place_t) noexcept
		: m_value{}
	{}

	template<typename... Ts>
		requires std::is_constructible_v<ValueType, Ts...>
	constexpr explicit result(std::in_place_t, Ts&&... args)
	noexcept(std::is_nothrow_constructible_v<ValueType, Ts...>)
		: m_value{std::forward<Ts>(args)...}
	{}

	template<typename T, typename... Ts>
		requires m_is_constructible_with_il_v<ValueType, T, Ts...>
	constexpr explicit result(std::in_place_t,
				  std::initializer_list<T> list,
				  Ts&&...		   args)
	noexcept(m_is_nothrow_constructible_with_il_v<ValueType, T, Ts...>)
		: m_value{list, std::forward<Ts>(args)...}
	{}

	template<typename... Ts>
		requires std::is_constructible_v<ErrorType, Ts...>
	constexpr explicit result(err_t, Ts&&... args)
	noexcept(std::is_nothrow_constructible_v<ErrorType, Ts...>)
		: m_value{std::forward<Ts>(args)...}
	{}

	template<typename T, typename... Ts>
		requires m_is_constructible_with_il_v<ErrorType, T, Ts...>
	constexpr explicit result(
		err_t,
		std::initializer_list<T> list,
		Ts&&...			 args)
	noexcept(m_is_nothrow_constructible_with_il_v<ErrorType, T, Ts...>)
		: m_value{list, std::forward<Ts>(args)...}
	{}

	constexpr ~result() = default;

	constexpr ~result()
	noexcept(conjunction_v<std::is_nothrow_destructible>)
		requires disjunction_v<std::is_trivially_destructible>
	{
		if (this->m_has_value)
			std::destroy_at(this->m_value);
		else std::destroy_at(this->m_error);
	}

	result& operator=(const result&) = delete;

	constexpr result& operator=(const result& other)
	noexcept(m_is_nothrow_copy_or_move_assignable_v)
		requires m_is_copy_or_move_assignable_v
	{
		if (other.m_has_value)
			this->m_assign_value(other.m_value);
		else this->m_assign_error(other.m_error);
		return *this;
	}

	constexpr result& operator=(result&& other)
	noexcept(m_is_nothrow_copy_or_move_assignable_v)
		requires m_is_copy_or_move_assignable_v
	{
		if (other.m_has_value)
			this->m_assign_value(std::move(other.m_value));
		else this->m_assign_error(std::move(other.m_error));
		return *this;
	}

	template<typename T = ValueType>
		requires (!std::is_same_v<std::remove_cvref_t<T>, result>)
			&& (!is_error_v<std::remove_cvref_t<T>>)
			&& std::is_constructible_v<ValueType, T>
			&& std::is_assignable_v<ValueType&, T>
			&& (std::is_nothrow_constructible_v<ValueType, T>
				|| disjunction_v<std::is_nothrow_move_constructible>)
	constexpr result& operator=(T&& value)
	{
		this->m_assign_value(std::forward<T>(value));
		return *this;
	}

	template<typename T>
		requires m_is_assignable_with_error_v<T>
	constexpr result& operator=(const error<T>& error)
	{
		this->m_assign_error(error.error());
		return *this;
	}

	template<typename T>
		requires m_is_assignable_with_error_v<T>
	constexpr result& operator=(error<T>&& error)
	{
		this->m_assign_error(std::move(error).error());
		return *this;
	}

	constexpr operator bool() const noexcept
	{ return this->m_has_value; }

	[[nodiscard]]
	constexpr bool has_value() const noexcept
	{ return this->m_has_value; }

	[[nodiscard]]
	constexpr const ValueType* operator->() noexcept
	{ return this->m_value; }

	[[nodiscard]]
	constexpr const ValueType* operator->() const noexcept
	{ return this->m_value; }

	[[nodiscard]]
	constexpr ValueType& operator*() & noexcept
	{ return this->m_value; }

	[[nodiscard]]
	constexpr const ValueType& operator*() const& noexcept
	{ return this->m_value; }

	[[nodiscard]]
	constexpr ValueType&& operator*() && noexcept
	{ return this->m_value; }

	[[nodiscard]]
	constexpr ValueType& value() &
	{
		if (!this->m_has_value) [[unlikely]]
			U_THROW(bad_result_access{this->m_error});
		return this->m_value;
	}

	[[nodiscard]]
	constexpr const ValueType& value() const&
	{
		if (!this->m_has_value) [[unlikely]]
			U_THROW(bad_result_access{this->m_error});
		return this->m_value;
	}

	[[nodiscard]]
	constexpr ValueType&& value() &&
	{
		if (!this->m_has_value) [[unlikely]]
			U_THROW(bad_result_access{std::move(this->m_error)});
		return this->m_value;
	}

	[[nodiscard]]
	constexpr const ValueType&& value() const&&
	{
		if (!this->m_has_value) [[unlikely]]
			U_THROW(bad_result_access{std::move(this->m_error)});
		return this->m_value;
	}

	template<std::convertible_to<ValueType> T>
	[[nodiscard]]
	constexpr ValueType value_or(T&& other_value) const&
	noexcept(std::is_nothrow_copy_constructible_v<ValueType>
		&& std::is_nothrow_convertible_v<T, ValueType>)
		requires std::is_copy_constructible_v<ValueType>
	{
		if (this->m_has_value)
			return this->m_value;
		return static_cast<ValueType>(std::forward<T>(other_value));
	}

	template<std::convertible_to<ValueType> T>
	[[nodiscard]]
	constexpr ValueType value_or(T&& other_value) &&
		noexcept(std::is_nothrow_move_constructible_v<ValueType>
			&& std::is_nothrow_convertible_v<T, ValueType>)
		requires std::is_move_constructible_v<ValueType>
	{ 
		if (this->m_has_value)
			return std::move(this->m_value);
		return static_cast<ValueType>(std::forward<T>(other_value));
	} 

	[[nodiscard]]
	constexpr ErrorType& error() & noexcept
	{ return this->m_error; }

	[[nodiscard]]
	constexpr ErrorType const& error() const&
	{ return this->m_error; }

	[[nodiscard]]
	constexpr ErrorType&& error() &&
	{ return std::move(this->m_error); }

	[[nodiscard]]
	constexpr const ErrorType&& error() const&&
	{ return std::move(this->m_error); }

private:
	union {
		ValueType m_value;
		ErrorType m_error;
	};
	bool m_has_value{true};

	template<typename T>
	constexpr void m_assign_value(T&& value) noexcept
	{
		if (!m_has_value) {
			detail::result_helpers::reconstruct(
				std::addressof(this->m_value),
				std::addressof(this->m_error),
				std::forward<T>(value));
			m_has_value = true;
		} else m_value = std::forward<T>(value);
	}

	template<typename T>
	constexpr void m_assign_error(T&& error) noexcept
	{
		if (m_has_value) {
			detail::result_helpers::reconstruct(
				std::addressof(this->m_value),
				std::addressof(this->m_error),
				std::forward<T>(error));
			m_has_value = false;
		} else m_error = std::forward<T>(error);
	}
};

}
