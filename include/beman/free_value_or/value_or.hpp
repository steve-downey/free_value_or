// value_or.hpp                                                       -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR
#define INCLUDED_BEMAN_FREE_VALUE_OR_VALUE_OR

#include <iterator>

namespace smd {
namespace free_value_or {

template <class T>
concept nullable = requires(const T t) {
    bool(t);
    *(t);
};

template <nullable T, class U, class R>
constexpr auto reference_or(T&& m, U&& u) -> R;

template <nullable T, class U, class R>
constexpr auto value_or(T&& m, U&& u) -> R;

template <nullable T, class I, class R>
constexpr auto or_invoke(T&& m, I&& invocable) -> R;
}
}

template <smd::free_value_or::nullable T, class U, class R = std::common_reference_t<std::iter_reference_t<T>, U&&>>
constexpr auto smd::free_value_or::reference_or(T&& m, U&& u) -> R {
    static_assert(!std::reference_constructs_from_temporary_v<R, U>);
    static_assert(!std::reference_constructs_from_temporary_v<R, T&>);

    return bool(m) ? static_cast<R>(*m) : static_cast<R>((U&&)u);
}

template <smd::free_value_or::nullable T, class U, class R = std::common_type_t<std::iter_reference_t<T>, U&&>>
constexpr auto smd::free_value_or::value_or(T&& m, U&& u) -> R {
    return bool(m) ? static_cast<R>(*m) : static_cast<R>(std::forward<U>(u));
}

template <smd::free_value_or::nullable T, class I, class R = std::common_type_t<std::iter_reference_t<T>, std::invoke_result_t<I>>>
constexpr auto smd::free_value_or::or_invoke(T&& m, I&& invocable) -> R {
    return bool(m) ? static_cast<R>(*m) : static_cast<R>(invocable());
}

#endif
