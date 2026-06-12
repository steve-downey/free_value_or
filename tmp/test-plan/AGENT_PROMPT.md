# Agent Prompt — `free_value_or` test build-out

You are a fresh agent picking up a multi-step task to build a comprehensive test suite for
the `free_value_or` library. You have **no prior context**; everything you need is on disk.
Work one step, then hand off to the next fresh agent. Do not try to do the whole plan.

## Absolute paths

- Project (main checkout, on branch `main`): `/home/sdowney/src/free_value_or/free_value_or`
- Plan dir (git-ignored, main checkout only): `/home/sdowney/src/free_value_or/free_value_or/tmp/test-plan`
  - Master plan: `tmp/test-plan/PLAN.md`
  - Checklist / state: `tmp/test-plan/CHECKLIST.md`
  - Steps: `tmp/test-plan/steps/step-NN-*.md`
  - Handoffs: `tmp/test-plan/handoff/handoff-NNN-*.md`

> The plan dir is reachable by absolute path from any worktree. Your code work happens in a
> worktree elsewhere; your *bookkeeping* (checklist, handoff) always writes back to the
> absolute plan-dir paths above.

## Startup procedure (do this in order, every time)

1. **Read the latest handoff.** List `tmp/test-plan/handoff/`, open the highest-numbered
   `handoff-*.md`. This is the single most important file — it carries hard-won, current
   knowledge (toolchain quirks, what actually compiled, surprises). Trust it over your
   assumptions.
2. **Read `tmp/test-plan/CHECKLIST.md`.** The first unchecked step is *your* step. Confirm
   it matches what the handoff says is next. If they disagree, the handoff wins — note the
   discrepancy in your own handoff.
3. **Read `tmp/test-plan/PLAN.md`** (orientation: semantics, matrix, standard-version
   reality, guardrails).
4. **Read your step file** `tmp/test-plan/steps/step-NN-*.md` — the detailed instructions.

## Execution procedure

5. Create a worktree + branch off `main` for your step:
   ```bash
   cd /home/sdowney/src/free_value_or/free_value_or
   git worktree add -b test/step-NN ../fvo-step-NN main
   cd ../fvo-step-NN
   ```
   (Pick `NN` = your step number. If the branch/worktree already exists from a failed prior
   attempt, the handoff should say so; clean it up first — see Recovery below.)
6. Do the work described in your step file. Build and run tests with the commands the step
   gives. **Iterate until green** (or, for negative-compile steps, until the WILL_FAIL
   tests pass for the right reason). Do not weaken a test just to pass it.
7. **Guardrail:** never edit `include/beman/free_value_or/value_or.hpp` to make a test
   pass. If you find a genuine header bug, record it under "Issues" in `CHECKLIST.md` and
   in your handoff, and continue (test around it or mark the case `// KNOWN ISSUE:`).

## Wrap-up procedure

8. Commit in the worktree:
   ```bash
   git add -A && git commit -m "test(step-NN): <summary>"
   ```
   End the commit message body with:
   `Co-Authored-By: Claude Opus 4.8 <noreply@anthropic.com>`
9. Merge into `main` and clean up the worktree:
   ```bash
   git -C /home/sdowney/src/free_value_or/free_value_or merge --no-ff test/step-NN \
     -m "Merge test/step-NN"
   git worktree remove ../fvo-step-NN
   git branch -d test/step-NN
   ```
   Confirm `main` builds the full suite after the merge (re-run the step's build once from
   the main checkout).
10. Update `tmp/test-plan/CHECKLIST.md`: tick your step, fill in any "Issues".
11. Write `tmp/test-plan/handoff/handoff-NNN-<slug>.md` (NNN = next zero-padded index) using
    `tmp/test-plan/handoff/TEMPLATE.md`. Be concrete: what you added, exact build commands
    that worked, toolchain facts you discovered, anything that bit you, and what the next
    agent should watch for.
12. **Stop.** Report a one-paragraph summary to the human and end. Do not start the next
    step.

## Recovery / idempotency

- Leftover worktree: `git worktree remove --force ../fvo-step-NN` then re-add.
- Leftover branch from a half-done step that never merged: inspect with `git log
  test/step-NN`; either reuse it or `git branch -D` and start fresh. Record what you did.
- If `main` is mid-step (partial work merged), the handoff should explain; finish forward,
  don't revert merged commits without saying so.

## Conventions (match existing code)

- Tests: Catch2 v3. `find_package(Catch2 3 REQUIRED)`, link `Catch2::Catch2WithMain`,
  register with `catch_discover_tests`. One executable per `.test.cpp` file.
- Every new file starts with `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.
- Test files live in `tests/beman/free_value_or/`. Headers shared by tests too.
- **Namespace alias (rename-proofing):** the library namespace is `smd::free_value_or` today
  but will be renamed to `beman::free_value_or` later (the "great renaming", before
  upstreaming). Never spell `smd::` inline in tests. Instead refer to the functions via the
  `fvo` alias that `test_types.hpp` defines (`namespace fvo = smd::free_value_or;  // rename
  point: smd:: -> beman::`), i.e. write `fvo::value_or`, `fvo::nullable`, etc. The future
  rename then touches one line. For a negative-compile TU that can't include `test_types.hpp`,
  put the same one-line alias at the top of the TU.
- Build at **C++23 minimum** (see PLAN §2). `optional<T&>` work is C++26 + feature-gated.
- Run `git -C <main> status` clean of `tmp/` (it's ignored) before merging.
