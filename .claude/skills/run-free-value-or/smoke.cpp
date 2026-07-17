// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Direct-compile smoke driver for beman.free_value_or (header-only).
// Exercises value_or / reference_or / or_invoke / or_construct against every
// nullable model type, with no CMake / Catch2 / network dependency.
#include <beman/free_value_or/value_or.hpp>

#include <cassert>
#include <expected>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <cstdio>

namespace fvo = smd::free_value_or;

int main() {
    // ---- value_or: engaged / disengaged across nullable models ----
    std::optional<int>      o_eng{42}, o_dis{};
    std::expected<int, int> e_eng{7}, e_dis{std::unexpected(0)};
    int                     obj   = 5;
    int *                   p_eng = &obj, *p_dis = nullptr;
    auto                    s_eng = std::make_shared<int>(9);
    std::shared_ptr<int>    s_dis{};

    assert(fvo::value_or(o_eng, 0) == 42);
    assert(fvo::value_or(o_dis, 0) == 0);
    assert(fvo::value_or(e_eng, 0) == 7);
    assert(fvo::value_or(e_dis, 0) == 0);
    assert(fvo::value_or(p_eng, 0) == 5);
    assert(fvo::value_or(p_dis, 0) == 0);
    assert(fvo::value_or(s_eng, 0) == 9);
    assert(fvo::value_or(s_dis, 0) == 0);
    assert(fvo::value_or(std::make_unique<int>(11), 0) == 11); // move-only rvalue

    // common_type promotion
    static_assert(std::is_same_v<decltype(fvo::value_or(o_eng, 1L)), long>);
    assert(fvo::value_or(o_dis, 99L) == 99L);

    // ---- reference_or: returns a reference into the engaged object ----
    std::optional<int> r{100};
    int                fallback = -1;
    decltype(auto)     ref      = fvo::reference_or(r, fallback);
    static_assert(std::is_reference_v<decltype(ref)>);
    assert(&ref == &*r);

    // ---- or_invoke: lazy fallback only runs when disengaged ----
    int  calls = 0;
    auto make  = [&] {
        ++calls;
        return -7;
    };
    assert(fvo::or_invoke(o_eng, make) == 42); // engaged: not called
    assert(calls == 0);
    assert(fvo::or_invoke(o_dis, make) == -7); // disengaged: called
    assert(calls == 1);

    // ---- or_construct: build the fallback in place ----
    std::optional<std::string> so_dis{};
    assert(fvo::or_construct<std::string>(so_dis, "built") == "built");
    std::optional<std::string> so_eng{"kept"};
    assert(fvo::or_construct<std::string>(so_eng, "built") == "kept");
    // initializer_list overload
    assert(fvo::or_construct<std::string>(so_dis, {'a', 'b', 'c'}) == "abc");

    std::puts("free_value_or smoke: ALL CHECKS PASSED");
    return 0;
}
