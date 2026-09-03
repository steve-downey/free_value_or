// tests/beman/expected/testing/types.hpp                            -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef BEMAN_EXPECTED_TESTING_TYPES_HPP
#define BEMAN_EXPECTED_TESTING_TYPES_HPP

#include <beman/expected/test_expected.hpp>

#include <initializer_list>
#include <utility>

namespace beman::expected::testing {

// Non-trivially destructible, copyable, and movable.
// Triggers explicit destroy_at / construct_at paths in unions.
struct traced {
    int val;

    constexpr explicit traced(int v = 0) noexcept : val(v) {}
    constexpr traced(const traced& o) noexcept : val(o.val) {}
    constexpr traced(traced&& o) noexcept : val(o.val) { o.val = -1; }
    constexpr traced& operator=(const traced& o) {
        val = o.val;
        return *this;
    }
    constexpr traced& operator=(traced&& o) noexcept {
        val   = o.val;
        o.val = -1;
        return *this;
    }
    constexpr ~traced() { val = -999; }

    constexpr bool operator==(const traced& o) const { return val == o.val; }
};

static_assert(!std::is_trivially_destructible_v<traced>);
static_assert(!std::is_trivially_copy_constructible_v<traced>);
static_assert(!std::is_trivially_move_constructible_v<traced>);

// Move-only, non-trivially destructible.
// Forces move-constructor and move-assignment paths; copy is deleted.
struct move_only {
    int val;

    constexpr explicit move_only(int v = 0) : val(v) {}
    constexpr move_only(move_only&& o) noexcept : val(o.val) { o.val = -1; }
    constexpr move_only& operator=(move_only&& o) noexcept {
        val   = o.val;
        o.val = -1;
        return *this;
    }
    move_only(const move_only&)            = delete;
    move_only& operator=(const move_only&) = delete;
    constexpr ~move_only() { val = -999; }

    constexpr bool operator==(const move_only& o) const { return val == o.val; }
};

static_assert(!std::is_copy_constructible_v<move_only>);
static_assert(std::is_nothrow_move_constructible_v<move_only>);

// Accepts an initializer_list in its constructor.
// Triggers the in_place_t(initializer_list<U>, Args...) and
// unexpect_t(initializer_list<U>, Args...) constructor paths.
struct init_list_type {
    int sum;
    int count;

    constexpr init_list_type(std::initializer_list<int> il, int extra = 0) noexcept : sum(extra), count(0) {
        for (int v : il) {
            sum += v;
            ++count;
        }
    }
    constexpr init_list_type(const init_list_type&)            = default;
    constexpr init_list_type(init_list_type&&)                 = default;
    constexpr init_list_type& operator=(const init_list_type&) = default;
    constexpr init_list_type& operator=(init_list_type&&)      = default;
    constexpr ~init_list_type()                                = default;

    constexpr bool operator==(const init_list_type& o) const { return sum == o.sum && count == o.count; }
};

// Convertible pair for testing converting constructors.
// widened is constructible from narrowed (implicit), triggering
// expected<widened, widened> from expected<narrowed, narrowed>.
struct narrowed {
    int val;
    constexpr explicit narrowed(int v = 0) : val(v) {}
    constexpr bool operator==(const narrowed& o) const { return val == o.val; }
};

struct widened {
    long val;
    constexpr widened(narrowed n) : val(n.val) {}
    constexpr explicit widened(long v) : val(v) {}
    constexpr bool operator==(const widened& o) const { return val == o.val; }
};

// Derived from expected — tests constraint 23.6: if T is cv bool,
// remove_cvref_t<U> must not be a specialization of expected.
// Also tests that value ctor rejects expected-derived types.
struct from_expected : test_ns::expected<int, int> {
    using test_ns::expected<int, int>::expected;
};

// Derived from unexpected — tests Mandate:
// T must not be a specialization of unexpected.
struct from_unexpected : test_ns::unexpected<int> {
    using test_ns::unexpected<int>::unexpected;
};

// Cross-type equality pair.
// eq_a and eq_b are equality-comparable to each other but are distinct types,
// enabling cross-type expected<T, eq_a> == expected<T, eq_b> tests.
struct eq_a {
    int val;
    constexpr explicit eq_a(int v = 0) : val(v) {}
    constexpr bool operator==(const eq_a& o) const { return val == o.val; }
};

struct eq_b {
    int val;
    constexpr explicit eq_b(int v = 0) : val(v) {}
    constexpr bool        operator==(const eq_b& o) const { return val == o.val; }
    friend constexpr bool operator==(const eq_a& a, const eq_b& b) { return a.val == b.val; }
};

} // namespace beman::expected::testing

#endif // BEMAN_EXPECTED_TESTING_TYPES_HPP
