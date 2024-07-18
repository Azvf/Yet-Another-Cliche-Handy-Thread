#pragma once

#include <type_traits>
#include <utility>
#include <cassert>

namespace ustl {
	template <class Ty>
	using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<Ty>>;
	
	template <class _Type, template <class...> class _Template>
	_INLINE_VAR constexpr bool is_specialization_v = false; // true if and only if _Type is a specialization of _Template
	template <template <class...> class _Template, class... _Types>
	_INLINE_VAR constexpr bool is_specialization_v<_Template<_Types...>, _Template> = true;

	template <class _Type, template <class...> class _Template>
	struct is_specialization : std::bool_constant<is_specialization_v<_Type, _Template>> {};

}