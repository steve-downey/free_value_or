# Handoff NNN — after Step NN (<slug>)

**Author:** Step NN agent · **Date:** YYYY-MM-DD · **Branch merged:** `test/step-NN`

## What I did
- (bullet list of files added/changed and what they cover)

## Build & test commands that actually worked
```bash
# paste the exact, verified commands — config + build + ctest
```
Result: (e.g. "47 assertions in 9 test cases, all pass"; for negative steps, which
WILL_FAIL targets pass and the regex used.)

## Toolchain / standard facts I confirmed
- (compiler version, what C++23/26 features compiled, `optional<T&>` availability, feature
  macro values, libstdc++ surprises). Update `CHECKLIST.md` "Toolchain facts" too.

## Gotchas / things that bit me
- (anything non-obvious: a header that needed including, a `common_reference` result that
  surprised, a diagnostic-regex that was compiler-specific, a CMake quirk)

## Issues found (if any)
- (suspected header bugs — also logged in CHECKLIST.md "Issues". Did NOT modify the header.)

## State of `main`
- (does the full suite build & pass from the main checkout? any target excluded/skipped?)

## What the next agent (Step NN+1) should know
- (specific advice, files to reuse, pitfalls to avoid)
