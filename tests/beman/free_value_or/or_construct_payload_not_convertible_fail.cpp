// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Must NOT compile: std::string is not convertible to int.
// The explicit Ret=int causes the engaged path to do static_cast<int>(*m), which is ill-formed.
// This is a Mandates-style hard error (not a constraint failure) — the intended stable diagnostic
// is "invalid 'static_cast'" (not "no matching function for call to").

#include <beman/free_value_or/value_or.hpp>
#include <optional>
#include <string>

void check() { (void)smd::free_value_or::or_construct<int>(std::optional<std::string>{}, 0); }
