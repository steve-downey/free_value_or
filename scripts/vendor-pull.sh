#!/usr/bin/env bash
# scripts/vendor-pull.sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Pull upstream changes into the co-located, non-squash vendored subtrees
# (beman::optional, beman::expected). See scripts/vendor.conf for the mapping.
#
# Usage:
#   scripts/vendor-pull.sh [lib ...]     # default: every lib in vendor.conf
#
# Mechanism: `git subtree split` flattens an upstream subdirectory (e.g.
# include/beman/optional) into a temporary root-rooted history, which is then
# `git subtree merge`d into the matching local prefix. This is the read side of
# the round-trip whose write side is scripts/vendor-push.sh.
#
# After a successful pull the per-lib baseline tag (vendor-base/<lib>) is moved
# to the new state, so vendor-push.sh exports only edits made *after* this sync.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
conf="$root/scripts/vendor.conf"
cd "$root"

if [ ! -f "$conf" ]; then echo "error: $conf not found" >&2; exit 1; fi

if ! git diff --quiet || ! git diff --cached --quiet; then
    echo "error: working tree is dirty; commit or stash before pulling." >&2
    exit 1
fi

want=("$@")
wanted() {
    [ ${#want[@]} -eq 0 ] && return 0
    local l="$1" w
    for w in "${want[@]}"; do [ "$w" = "$l" ] && return 0; done
    return 1
}

# Read the config into arrays (portable, no associative-array requirement).
libs=(); urls=(); branches=(); prefixes=()
while read -r lib url branch prefix _rest; do
    case "$lib" in ''|'#'*) continue;; esac
    wanted "$lib" || continue
    libs+=("$lib"); urls+=("$url"); branches+=("$branch"); prefixes+=("$prefix")
done < "$conf"

if [ ${#libs[@]} -eq 0 ]; then echo "nothing to do (no matching libs)"; exit 0; fi

# Ensure remotes exist and fetch each once.
fetched=" "
for i in "${!libs[@]}"; do
    lib="${libs[$i]}"; url="${urls[$i]}"; branch="${branches[$i]}"
    remote="beman-$lib"
    if ! git remote get-url "$remote" >/dev/null 2>&1; then
        echo ">> adding remote $remote -> $url"
        git remote add "$remote" "$url"
    fi
    if [[ "$fetched" != *" $remote "* ]]; then
        echo ">> fetching $remote ($branch)"
        git -c submodule.recurse=false fetch --no-tags "$remote" "$branch"
        fetched="$fetched$remote "
    fi
done

# Pull each prefix: flatten upstream subdir, then subtree-merge it in.
touched=" "
for i in "${!libs[@]}"; do
    lib="${libs[$i]}"; branch="${branches[$i]}"; prefix="${prefixes[$i]}"
    remote="beman-$lib"
    tmp="refs/heads/_vendor-pull-tmp"
    echo ">> pull $prefix <- $remote/$branch"
    git update-ref -d "$tmp" 2>/dev/null || true
    # Suppress subtree split's very noisy per-commit progress.
    git subtree split -P "$prefix" "$remote/$branch" -b _vendor-pull-tmp >/dev/null 2>&1
    git subtree merge -P "$prefix" _vendor-pull-tmp \
        -m "vendor-pull: $prefix from $remote/$branch" >/dev/null
    git branch -D _vendor-pull-tmp >/dev/null 2>&1 || true
    touched="$touched$lib "
done

# Move baseline tags forward for every lib we touched.
for lib in $(echo "$touched" | tr ' ' '\n' | sort -u); do
    [ -z "$lib" ] && continue
    git tag -f "vendor-base/$lib" HEAD >/dev/null
    echo ">> baseline vendor-base/$lib -> $(git rev-parse --short HEAD)"
done

echo "done. Review merges with: git log --oneline --graph -8"
