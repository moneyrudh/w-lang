#!/usr/bin/env bash
# W Lang test runner. Tests are grouped by language feature (the sections of
# docs/LANGUAGE.md), e.g. tests/run/control_flow/, tests/errors/loops/.
#
#   tests/run/**/NAME.w     must transpile, compile with -Wall -Wextra -Werror, and run.
#                           stdout must match NAME.out exactly; exit code must match
#                           the "-- exit: N" line in NAME.w (default 0).
#   tests/errors/**/NAME.w  must fail to transpile (exit 1). Every "-- error: TEXT"
#                           line must appear in stderr, and there must be no other
#                           errors (catches cascades).
#
# Usage: tests/run.sh [filter]     (runs tests whose path contains filter,
#                                   e.g. "loops" or "elf")
# Env:   TRANSPILER=path           (default ./build/transpiler)

set -u

cd "$(dirname "$0")/.." || exit 1

TRANSPILER="${TRANSPILER:-./build/transpiler}"
CC="${CC:-gcc}"
FILTER="${1:-}"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

if [ ! -x "$TRANSPILER" ]; then
    echo "transpiler not found at $TRANSPILER (run make first)"
    exit 1
fi

pass=0
fail=0
failures=()

report_fail() {
    fail=$((fail + 1))
    failures+=("$1")
    echo "FAIL  $1"
    sed 's/^/      /' "$WORK/reason"
}

run_test() {
    local src="$1" name expected_exit
    name="${src%.w}"
    expected_exit="$(sed -n 's/^-- exit: *\([0-9][0-9]*\).*/\1/p' "$src" | head -1)"
    expected_exit="${expected_exit:-0}"

    if ! "$TRANSPILER" "$src" "$WORK/out.c" >"$WORK/transpile.log" 2>&1; then
        { echo "transpiler failed:"; cat "$WORK/transpile.log"; } >"$WORK/reason"
        report_fail "$src"; return
    fi
    if ! "$CC" -Wall -Wextra -Werror "$WORK/out.c" -o "$WORK/prog" >"$WORK/cc.log" 2>&1; then
        { echo "generated C failed to compile:"; head -20 "$WORK/cc.log"; } >"$WORK/reason"
        report_fail "$src"; return
    fi

    "$WORK/prog" >"$WORK/stdout" 2>&1
    local actual_exit=$?

    if [ ! -f "$name.out" ]; then
        echo "missing expected output file $name.out" >"$WORK/reason"
        report_fail "$src"; return
    fi
    if ! diff -u "$name.out" "$WORK/stdout" >"$WORK/diff"; then
        { echo "stdout differs (- expected, + actual):"; tail -n +3 "$WORK/diff"; } >"$WORK/reason"
        report_fail "$src"; return
    fi
    if [ "$actual_exit" != "$expected_exit" ]; then
        echo "exit code $actual_exit, expected $expected_exit" >"$WORK/reason"
        report_fail "$src"; return
    fi

    pass=$((pass + 1))
    echo "ok    $src"
}

error_test() {
    local src="$1"
    "$TRANSPILER" "$src" "$WORK/out.c" >"$WORK/stdout" 2>"$WORK/stderr"
    local status=$?

    : >"$WORK/reason"
    if [ "$status" != 1 ]; then
        echo "transpiler exit code $status, expected 1" >>"$WORK/reason"
    fi

    local expected_count=0
    while IFS= read -r expected; do
        expected_count=$((expected_count + 1))
        if ! grep -qF -- "$expected" "$WORK/stderr"; then
            echo "missing error: $expected" >>"$WORK/reason"
        fi
    done < <(sed -n 's/^-- error: *//p' "$src")

    if [ "$expected_count" = 0 ]; then
        echo "no '-- error:' lines in test file" >>"$WORK/reason"
    fi

    local actual_count
    actual_count="$(grep -c '^Error' "$WORK/stderr")"
    if [ "$actual_count" != "$expected_count" ]; then
        echo "$actual_count errors reported, expected $expected_count" >>"$WORK/reason"
    fi

    if [ -s "$WORK/reason" ]; then
        { echo "stderr:"; sed 's/^/  /' "$WORK/stderr"; } >>"$WORK/reason"
        report_fail "$src"; return
    fi

    pass=$((pass + 1))
    echo "ok    $src"
}

while IFS= read -r src; do
    case "$src" in *"$FILTER"*) run_test "$src" ;; esac
done < <(find tests/run -name '*.w' | sort)
while IFS= read -r src; do
    case "$src" in *"$FILTER"*) error_test "$src" ;; esac
done < <(find tests/errors -name '*.w' | sort)

echo
echo "$pass passed, $fail failed"
if [ "$fail" -gt 0 ]; then
    for f in "${failures[@]}"; do echo "  $f"; done
    exit 1
fi
