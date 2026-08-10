// tests/beman/expected/testing/constant_eval.hpp                    -*-C++-*-
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// Reporting compile-time facts through the runtime test framework.
//
// The usual way to test a constexpr contract is `static_assert`. That has one
// bad property as a *test*: a wrong answer is a translation failure, so the
// only thing anyone ever sees is a compiler diagnostic. The test run reports
// nothing, the xUnit output is empty, and because the build stops at the
// first failing assertion no other test in the file is exercised at all.
//
// `constant_eval` separates the two questions a constexpr test actually asks:
//
//   1. "can this be constant-evaluated at all?" — still a hard translation
//      failure, because that is a property of the code, not of a value.
//   2. "does it produce the right answer?" — an ordinary runtime comparison
//      inside a TEST_CASE, so a wrong answer is *reported*, with the actual
//      and expected values, alongside every other test in the run.
//
// `constant_eval` is `consteval`, so a call to it is an immediate invocation:
// the probe is evaluated during translation and its result is required to be
// a constant expression. Question 1 is therefore answered by the call itself,
// with no `static_assert` needed — if the probe body is not usable in a
// constant expression, the program is ill-formed. The result then behaves as
// an ordinary prvalue, free to be compared by CHECK.
//
//   TEST_CASE("expected: constexpr error construction") {
//       constexpr auto probe = [] {
//           expt::expected<int, int> e(expt::unexpect, 7);
//           return int_state{e.has_value(), e.error()};
//       };
//       CHECK(constant_eval(probe) == int_state{false, 7});
//   }
//
// A probe is a plain lambda, not a `consteval` one, so the same probe body
// can be run in both evaluation modes — constant evaluation and ordinary
// evaluation can take different paths through a union-based type:
//
//   CHECK(constant_eval(probe) == expect);   // constant evaluation
//   CHECK(probe() == expect);                // ordinary evaluation
//
// Two constraints on a probe follow from the above, and neither is arbitrary:
//
//   - It takes no arguments and captures nothing. A closure over a runtime
//     value is not a constant expression. Construct whatever the probe
//     observes inside the probe.
//   - It returns a literal type by value — a scalar, or a small aggregate of
//     scalars — never a reference to something it created, and never a type
//     whose value cannot escape constant evaluation (`std::string`,
//     `std::vector`). Reduce such state to scalars inside the probe.
//
// Give the returned aggregate an `operator<<` so that a mismatch prints both
// states. Without one, Catch2 reports `{?} == {?}` and the reporting benefit
// this component exists for is lost.

#ifndef BEMAN_EXPECTED_TESTING_CONSTANT_EVAL_HPP
#define BEMAN_EXPECTED_TESTING_CONSTANT_EVAL_HPP

namespace beman::expected::testing {

// Constant-evaluate `probe` and return its result as an ordinary value.
template <class Probe>
consteval auto constant_eval(Probe probe);

} // namespace beman::expected::testing

template <class Probe>
consteval auto beman::expected::testing::constant_eval(Probe probe) {
    return probe();
}

#endif // BEMAN_EXPECTED_TESTING_CONSTANT_EVAL_HPP
