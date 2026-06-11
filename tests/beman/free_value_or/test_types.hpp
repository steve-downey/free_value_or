// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#ifndef FVO_TEST_TYPES_HPP
#define FVO_TEST_TYPES_HPP

#include <beman/free_value_or/value_or.hpp>

#include <expected>
#include <memory>
#include <optional>

// Rename-proofing alias: all tests use fvo:: rather than smd:: inline.
// When the library is renamed to beman::free_value_or, only this line changes.
namespace fvo = smd::free_value_or;  // rename point: smd:: -> beman::

// ---------------------------------------------------------------------------
// Feature detection for optional<T&> (P2988, C++26, needs newer libstdc++)
// ---------------------------------------------------------------------------
#if defined(__cpp_lib_optional) && __cpp_lib_optional >= 202406L
#define FVO_HAS_OPTIONAL_REF 1
#else
#define FVO_HAS_OPTIONAL_REF 0
#endif

// ---------------------------------------------------------------------------
// Helpers: make engaged / disengaged nullable values for each model type
// ---------------------------------------------------------------------------

template <class T>
struct NullableFixture {
    // optional<T>
    static std::optional<T> opt_engaged(T val) { return val; }
    static std::optional<T> opt_disengaged() { return std::nullopt; }

    // expected<T, int>
    static std::expected<T, int> exp_engaged(T val) { return val; }
    static std::expected<T, int> exp_disengaged() { return std::unexpected(0); }

    // shared_ptr<T>
    static std::shared_ptr<T> sptr_engaged(T val) {
        return std::make_shared<T>(val);
    }
    static std::shared_ptr<T> sptr_disengaged() { return nullptr; }

    // unique_ptr<T>
    static std::unique_ptr<T> uptr_engaged(T val) {
        return std::make_unique<T>(val);
    }
    static std::unique_ptr<T> uptr_disengaged() { return nullptr; }
};

// Raw pointer helpers (non-owning; caller manages the pointee lifetime)
template <class T>
T* raw_engaged(T& obj) { return &obj; }

template <class T>
T* raw_disengaged() { return nullptr; }

// ---------------------------------------------------------------------------
// Anti-model types: must NOT satisfy the nullable concept
// ---------------------------------------------------------------------------

// Has operator bool but no operator*
struct bool_only {
    explicit operator bool() const { return true; }
};

// Has operator* but no contextual bool conversion
struct deref_only {
    int value = 0;
    int& operator*() const { return const_cast<deref_only*>(this)->value; }
};

// Has operator bool and operator* but they are NON-CONST
// The nullable concept checks on a 'const T', so this must NOT satisfy it.
struct nonconst_nullable {
    int value = 42;
    explicit operator bool() /* non-const */ { return true; }
    int& operator*() /* non-const */ { return value; }
};

#endif  // FVO_TEST_TYPES_HPP
