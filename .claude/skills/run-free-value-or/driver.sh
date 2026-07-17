#!/usr/bin/env bash
# driver.sh — compile-and-run smoke driver for beman.free_value_or.
#
# free_value_or is a header-only C++ library; there is no app to launch.
# "Running" it means compiling a translation unit against
# include/beman/free_value_or/value_or.hpp and executing the checks.
#
# This driver compiles smoke.cpp (next to this script) at C++23 with
# -Iinclude and runs it. It needs ONLY a C++23 compiler — no CMake, no
# network, no Catch2, no vendored submodules. Use it as the fast inner loop
# when changing the header implementation (the layer most PRs touch).
#
# Usage:
#   .claude/skills/run-free-value-or/driver.sh            # uses g++
#   .claude/skills/run-free-value-or/driver.sh clang++    # pick a compiler
#   CXX=g++-14 .claude/skills/run-free-value-or/driver.sh # via env
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UNIT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"   # .claude/skills/run-free-value-or -> unit root
CXX="${1:-${CXX:-g++}}"
OUT="$(mktemp -d)/fvo_smoke"

echo "[driver] compiler : $CXX ($($CXX --version | head -1))"
echo "[driver] include  : $UNIT_ROOT/include"
echo "[driver] source   : $SCRIPT_DIR/smoke.cpp"

"$CXX" -std=c++23 -Wall -Wextra \
    -I"$UNIT_ROOT/include" \
    "$SCRIPT_DIR/smoke.cpp" -o "$OUT"

echo "[driver] compiled OK -> running"
"$OUT"
