#!/usr/bin/env bash
# scripts/vendor-push.sh
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
# Export local edits to a vendored library's co-located subtrees as a patch
# series ready to send upstream (bemanproject/optional, bemanproject/expected)
# for inclusion when the proposal is adopted.
#
# Usage:
#   scripts/vendor-push.sh <lib> [--since <ref>] [--out <dir>]
#
#   <lib>          optional | expected  (a lib named in scripts/vendor.conf)
#   --since <ref>  baseline to diff from (default: tag vendor-base/<lib>, i.e.
#                  the last vendor point). Edits after this ref are exported.
#   --out <dir>    output directory for the patch series (default: patches/<lib>)
#
# This script DELIBERATELY does not push anything. `git subtree push` is the
# wrong tool here: `git subtree split` flattens the prefix to the repo root, so
# the pushed tree would not match the upstream `include/beman/<lib>/...` layout.
# Because our prefixes are identical to the upstream paths, the emitted patches
# instead apply directly onto a clean upstream checkout:
#
#     cd /path/to/bemanproject-<lib>
#     git checkout -b free-value-or-value_or_construct
#     git am /path/to/patches/<lib>/*.patch     # preserves authorship/messages
#     # ...open a PR.
#
# Sending the changes upstream is a deliberate, manual step the maintainer takes.
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
conf="$root/scripts/vendor.conf"
cd "$root"

if [ $# -lt 1 ]; then
    sed -n '2,30p' "$0" | sed 's/^# \{0,1\}//'
    exit 2
fi

lib=""; since=""; out=""
while [ $# -gt 0 ]; do
    case "$1" in
        --since) since="$2"; shift 2;;
        --out)   out="$2";   shift 2;;
        --*)     echo "error: unknown option $1" >&2; exit 2;;
        *)       if [ -z "$lib" ]; then lib="$1"; shift; else
                     echo "error: unexpected argument $1" >&2; exit 2; fi;;
    esac
done
[ -n "$lib" ] || { echo "error: <lib> required" >&2; exit 2; }

# Collect this lib's prefixes from the config.
prefixes=()
while read -r clib _url _branch prefix _rest; do
    case "$clib" in ''|'#'*) continue;; esac
    [ "$clib" = "$lib" ] && prefixes+=("$prefix")
done < "$conf"
if [ ${#prefixes[@]} -eq 0 ]; then
    echo "error: no prefixes for lib '$lib' in $conf" >&2; exit 2
fi

# Default baseline: the vendor-base/<lib> tag written by vendoring / vendor-pull.
if [ -z "$since" ]; then
    if git rev-parse -q --verify "refs/tags/vendor-base/$lib" >/dev/null; then
        since="vendor-base/$lib"
    else
        echo "error: no --since given and tag vendor-base/$lib does not exist." >&2
        echo "       Pass --since <ref> (e.g. the subtree-add commit)." >&2
        exit 2
    fi
fi

out="${out:-patches/$lib}"

# Are there any edits to this lib's prefixes since the baseline?
if git diff --quiet "$since" HEAD -- "${prefixes[@]}"; then
    echo "no changes to $lib prefixes since $since; nothing to export."
    exit 0
fi

mkdir -p "$out"
rm -f "$out"/*.patch 2>/dev/null || true

echo ">> lib:      $lib"
echo ">> since:    $since ($(git rev-parse --short "$since"))"
echo ">> prefixes: ${prefixes[*]}"
echo ">> out:      $out"

# Per-commit patch series (cherry-pick/am friendly, preserves authorship).
git format-patch --output-directory "$out" "$since"..HEAD -- "${prefixes[@]}" \
    >/dev/null

# Also emit a single squashed net diff for quick review / git apply.
git diff "$since" HEAD -- "${prefixes[@]}" > "$out/NETDIFF-$lib.diff"

count=$(find "$out" -maxdepth 1 -name '*.patch' | wc -l | tr -d ' ')
echo ">> wrote $count patch(es) + NETDIFF-$lib.diff to $out/"
echo
echo "Apply onto a clean upstream checkout (paths already match):"
echo "    cd <bemanproject/$lib checkout>"
echo "    git checkout -b free-value-or-p3413"
echo "    git am $root/$out/*.patch"
