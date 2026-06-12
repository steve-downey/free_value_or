# Agent Prompt — `free_value_or` `or_construct` build-out

You are a fresh agent picking up a multi-step task to **add** a free `or_construct` function
to the `free_value_or` library and build a comprehensive test suite for it, mirroring the
matrix already applied to `value_or` / `reference_or` / `or_invoke`. You have **no prior
context**; everything you need is on disk. Work one step, then hand off to the next fresh
agent. Do not try to do the whole plan.

`or_construct` is the free-function analogue of P3413R0's member `value_or_construct` — the
one gap D4270R0 explicitly leaves open. See `PLAN.md` §0–§1 for the design and the exact
overloads.

## Absolute paths

- Project (main checkout, on branch `main`): `/home/sdowney/src/free_value_or/main`
  - **(The old sibling prompt's `…/free_value_or/free_value_or` path is stale — that is now
    the bare repo. Use `…/free_value_or/main`.)**
- This plan dir (git-ignored, main checkout only):
  `/home/sdowney/src/free_value_or/main/tmp/test-plan-or_construct`
  - Master plan: `PLAN.md`
  - Checklist / state: `CHECKLIST.md`
  - Steps: `steps/step-NN-*.md`
  - Handoffs: `handoff/handoff-NNN-*.md`
- Sibling plan (the three existing functions, for reference + toolchain facts):
  `/home/sdowney/src/free_value_or/main/tmp/test-plan`

> The plan dir is reachable by absolute path from any worktree. Your code work happens in a
> worktree elsewhere; your *bookkeeping* (checklist, handoff) always writes back to the
> absolute plan-dir paths above.

## Startup procedure (do this in order, every time)

1. **Read the latest handoff.** List `handoff/`, open the highest-numbered `handoff-*.md`.
   This is the single most important file — it carries hard-won, current knowledge
   (toolchain quirks, what actually compiled, surprises). Trust it over your assumptions.
2. **Read `CHECKLIST.md`.** The first unchecked step is *your* step. Confirm it matches what
   the handoff says is next. If they disagree, the handoff wins — note the discrepancy in
   your own handoff.
3. **Read `PLAN.md`** (orientation: the gap, the `or_construct` design, matrix, standard
   reality, guardrails).
4. **Read your step file** `steps/step-NN-*.md` — the detailed instructions.
5. **Skim the sibling plan's latest handoff** (`../test-plan/handoff/`, highest number) for
   the verified compiler version, build command, and the exact negative-compile diagnostic
   regexes — they apply here unchanged, so you don't re-derive them.

## Execution procedure

6. Create a worktree + branch off `main` for your step:
   ```bash
   cd /home/sdowney/src/free_value_or/main
   git worktree add -b or_construct/step-NN ../oc-step-NN main
   cd ../oc-step-NN
   ```
   (Pick `NN` = your step number. If the branch/worktree already exists from a failed prior
   attempt, the handoff should say so; clean it up first — see Recovery below. Do **not**
   reuse the unrelated leftover `fvo-step-04` worktree/branch.)
7. Do the work described in your step file. Build and run tests with the commands the step
   gives (see `PLAN.md` §2). **Iterate until green** (or, for the negative-compile step,
   until the WILL_FAIL tests pass for the right reason). Do not weaken a test just to pass
   it.
8. **Guardrail:** **Step 00 is the only step allowed to edit**
   `include/beman/free_value_or/value_or.hpp` (it adds the two `or_construct` overloads from
   `PLAN.md` §1.2). Every later step must NOT edit the header. If you find a genuine bug or
   design flaw, record it under "Issues" in `CHECKLIST.md` and in your handoff, and continue
   (test around it or mark the case `// KNOWN ISSUE:`). Never touch the three existing
   functions or their tests.

## Wrap-up procedure

9. Commit in the worktree:
   ```bash
   git add -A && git commit -m "test(or_construct step-NN): <summary>"
   ```
   (Step 00's commit is `feat(or_construct): add free or_construct overloads` + the smoke
   test.) End the commit message body with:
   `Co-Authored-By: Claude Opus 4.8 (1M context) <noreply@anthropic.com>`
10. Merge into `main` and clean up the worktree:
    ```bash
    git -C /home/sdowney/src/free_value_or/main merge --no-ff or_construct/step-NN \
      -m "Merge or_construct/step-NN"
    git worktree remove ../oc-step-NN
    git branch -d or_construct/step-NN
    ```
    Confirm `main` builds the full suite after the merge (re-run the build once from the
    main checkout) — the existing 46 tests **plus** your new ones must all pass.
11. Update `CHECKLIST.md`: tick your step, fill in any "Issues".
12. Write `handoff/handoff-NNN-<slug>.md` (NNN = next zero-padded index) using
    `handoff/TEMPLATE.md`. Be concrete: what you added, exact build commands that worked,
    toolchain facts you discovered, anything that bit you, and what the next agent should
    watch for.
13. **Stop.** Report a one-paragraph summary to the human and end. Do not start the next
    step.

## Recovery / idempotency

- Leftover worktree: `git worktree remove --force ../oc-step-NN` then re-add.
- Leftover branch from a half-done step that never merged: inspect with `git log
  or_construct/step-NN`; either reuse it or `git branch -D` and start fresh. Record what you
  did.
- If `main` is mid-step (partial work merged), the handoff should explain; finish forward,
  don't revert merged commits without saying so.

## Conventions (match existing code)

- Tests: Catch2 v3. Use the existing `fvo_add_test(<name> <source>)` /
  `fvo_add_compile_fail_test(<name> <source> <regex>)` helpers in
  `tests/beman/free_value_or/CMakeLists.txt`. One executable per `.test.cpp`.
- Every new file starts with `// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception`.
- Test files live in `tests/beman/free_value_or/`. Headers shared by tests too.
- **Namespace alias (rename-proofing):** the library namespace is `smd::free_value_or` today
  but will be renamed to `beman::free_value_or` later. Never spell `smd::` inline in tests.
  Refer to the function via the `fvo` alias that `test_types.hpp` defines — write
  `fvo::or_construct`. For a negative-compile TU that can't include `test_types.hpp`, put the
  same one-line alias at the top of the TU.
- Build at **C++23 minimum** (see `PLAN.md` §2). `optional<T&>` work is C++26 + feature-gated
  via `beman::optional` (`FVO_HAS_OPTIONAL_REF=1`, injected by `fvo_add_test`).
- Run `git -C <main> status` clean of `tmp/` (it's ignored) before merging.
