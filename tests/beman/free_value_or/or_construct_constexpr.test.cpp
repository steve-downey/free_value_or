// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "test_types.hpp"

#include <catch2/catch_test_macros.hpp>

#include <expected>
#include <optional>

// ---------------------------------------------------------------------------
// or_construct (pack overload) — constant-evaluated proofs
// ---------------------------------------------------------------------------

// engaged: returns held value, fallback args not used
static_assert(fvo::or_construct(std::optional<int>{7}, 0) == 7);
// disengaged: constructs int(5)
static_assert(fvo::or_construct(std::optional<int>{}, 5) == 5);
// disengaged, zero args: default-constructs int{}
static_assert(fvo::or_construct(std::optional<int>{}) == 0);

// std::expected — disengaged path
static_assert(fvo::or_construct(std::expected<int, int>{std::unexpected(0)}, 9) == 9);
// std::expected — engaged path
static_assert(fvo::or_construct(std::expected<int, int>{42}, 0) == 42);

// raw pointer: constexpr int* pointing at a constexpr object
static constexpr int ce_engaged = 3;
static_assert(fvo::or_construct(&ce_engaged, 0) == 3);
static_assert(fvo::or_construct(static_cast<const int*>(nullptr), 4) == 4);

// explicit Ret — engaged: static_cast<long>(*opt)
static_assert(fvo::or_construct<long>(std::optional<int>{7}, 0L) == 7L);
// explicit Ret — disengaged: constructs long(9)
static_assert(fvo::or_construct<long>(std::optional<int>{}, 9L) == 9L);

// ---------------------------------------------------------------------------
// or_construct (initializer_list overload) — constant-evaluated proofs
//
// std::vector / std::string allocate on the heap and cannot appear in
// static_assert context.  Use a tiny literal type whose initializer_list
// constructor is constexpr and non-allocating.
// ---------------------------------------------------------------------------

struct Sum {
    int total       = 0;
    constexpr Sum() = default;
    constexpr Sum(std::initializer_list<int> il) {
        for (int x : il)
            total += x;
    }
    constexpr bool operator==(const Sum&) const = default;
};

// disengaged: Sum is default-constructed then or_construct builds Sum{1,2,3}
static_assert(fvo::or_construct(std::optional<Sum>{}, {1, 2, 3}) == Sum{{1, 2, 3}});
// engaged: held Sum{} is returned; init-list args are not used
static_assert(fvo::or_construct(std::optional<Sum>{Sum{}}, {1, 2, 3}) == Sum{});

// ---------------------------------------------------------------------------
// Catch2 entry: the real proof is the static_asserts above.
// ---------------------------------------------------------------------------
TEST_CASE("or_construct constexpr static_asserts compile and hold", "[or_construct][constexpr]") { SUCCEED(); }
