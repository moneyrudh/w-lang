#!/usr/bin/env bash
# Tests of the wlang command itself: commands, options, exit codes, and which files
# each command writes. The language is tested by tests/run.sh.
#
# Every case runs in its own empty directory, so stray output files are caught.
#
# Usage: tests/driver/test.sh
# Env:   WLANG=path   (default ./build/wlang)

set -u

cd "$(dirname "$0")/../.." || exit 1
ROOT="$PWD"

WLANG="${WLANG:-./build/wlang}"
case "$WLANG" in /*) ;; *) WLANG="$ROOT/$WLANG" ;; esac

GOOD="$ROOT/tests/driver/exits_7.w"      # prints a line, exits 7
BROKEN="$ROOT/tests/driver/broken.w"     # one type error

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

if [ ! -x "$WLANG" ]; then
    echo "wlang not found at $WLANG (run make first)"
    exit 1
fi

pass=0
fail=0
failures=()

# ==================== helpers ====================

# starts a case in a fresh, empty directory
begin() {
    NAME="$1"
    CASE="$WORK/case$((pass + fail))"
    mkdir -p "$CASE" "$CASE.tmp"
    cd "$CASE" || exit 1
    : >"$WORK/reason"
}

# runs wlang with $TMPDIR pointed at a directory we can check afterwards
wl() {
    TMPDIR="$CASE.tmp" "$WLANG" "$@" >"$WORK/stdout" 2>"$WORK/stderr"
    STATUS=$?
}

problem() { echo "$1" >>"$WORK/reason"; }

expect_status() { [ "$STATUS" = "$1" ] || problem "exit code $STATUS, expected $1"; }
expect_stdout() { grep -qF -- "$1" "$WORK/stdout" || problem "stdout lacks: $1"; }
expect_stderr() { grep -qF -- "$1" "$WORK/stderr" || problem "stderr lacks: $1"; }
expect_file()   { [ -f "$1" ] || problem "missing file: $1"; }

# the case directory holds exactly these files (none when called without arguments)
expect_files() {
    local actual expected
    actual="$(cd "$CASE" && find . -type f | sed 's|^\./||' | sort | tr '\n' ' ')"
    expected="$(printf '%s\n' "$@" | sed '/^$/d' | sort | tr '\n' ' ')"
    [ "$actual" = "$expected" ] || problem "files written: [${actual% }], expected [${expected% }]"
}

# run and build must clean up their temporary directory
expect_temp_cleaned() {
    [ -z "$(ls -A "$CASE.tmp")" ] || problem "left behind in TMPDIR: $(ls -A "$CASE.tmp" | tr '\n' ' ')"
}

finish() {
    cd "$ROOT" || exit 1
    if [ -s "$WORK/reason" ]; then
        fail=$((fail + 1))
        failures+=("$NAME")
        echo "FAIL  driver: $NAME"
        sed 's/^/      /' "$WORK/reason"
        { echo "stdout:"; sed 's/^/  /' "$WORK/stdout"; echo "stderr:"; sed 's/^/  /' "$WORK/stderr"; } | sed 's/^/      /'
    else
        pass=$((pass + 1))
        echo "ok    driver: $NAME"
    fi
}

# ==================== run ====================

begin "bare file runs it"
wl "$GOOD"
expect_status 7; expect_stdout "hello from exits_7"; expect_files; expect_temp_cleaned
finish

begin "run passes on the exit code"
wl run "$GOOD"
expect_status 7; expect_stdout "hello from exits_7"; expect_files; expect_temp_cleaned
finish

begin "run accepts program arguments"
wl run "$GOOD" one two --three
expect_status 7; expect_files; expect_temp_cleaned
finish

begin "run with errors"
wl run "$BROKEN"
expect_status 1; expect_stderr "Error on line 3: Type mismatch"; expect_files; expect_temp_cleaned
finish

# ==================== build ====================

begin "build writes ./<name>"
wl build "$GOOD"
expect_status 0; expect_files exits_7; expect_temp_cleaned
"$CASE/exits_7" >/dev/null; [ $? = 7 ] || problem "built program didn't exit 7"
finish

begin "build -o"
wl build "$GOOD" -o hi
expect_status 0; expect_files hi; expect_temp_cleaned
finish

begin "build --emit-c keeps the C next to the executable"
wl build "$GOOD" -o hi --emit-c
expect_status 0; expect_files hi hi.c; expect_temp_cleaned
grep -q "int main" hi.c 2>/dev/null || problem "hi.c has no main()"
finish

begin "build --emit-c with a path"
wl build --emit-c out.c "$GOOD"
expect_status 0; expect_files exits_7 out.c
finish

begin "build with errors writes nothing"
wl build "$BROKEN" --emit-c
expect_status 1; expect_stderr "Error on line 3"; expect_files; expect_temp_cleaned
finish

# ==================== check ====================

begin "check a valid program"
wl check "$GOOD"
expect_status 0; expect_files
[ -s "$WORK/stdout" ] && problem "check printed to stdout"
finish

begin "check reports errors"
wl check "$BROKEN"
expect_status 1; expect_stderr "Error on line 3: Type mismatch"; expect_files
finish

# ==================== C compiler ====================

begin "run: options after the file belong to the program"
wl run "$GOOD" --cc no-such-cc-wlang
expect_status 7
finish

begin "missing --cc compiler"
wl run --cc no-such-cc-wlang "$GOOD"
expect_status 1; expect_stderr "C compiler 'no-such-cc-wlang' not found"; expect_temp_cleaned
finish

begin "CC from the environment"
CC=no-such-cc-wlang wl build "$GOOD"
expect_status 1; expect_stderr "C compiler 'no-such-cc-wlang' not found"; expect_files
finish

# ==================== command line ====================

begin "no arguments"
wl
expect_status 2; expect_stderr "Usage:"
finish

begin "unknown command"
wl compile "$GOOD"
expect_status 2; expect_stderr "unknown command 'compile'"
finish

begin "not a .w file"
wl build notes.txt
expect_status 2; expect_stderr "expected a .w file, got 'notes.txt'"
finish

begin "no file"
wl build -o hi
expect_status 2; expect_stderr "no .w file given"
finish

begin "-o only with build"
wl run -o hi "$GOOD"
expect_status 2; expect_stderr "-o only works with 'build'"
finish

begin "unknown option"
wl build --fast "$GOOD"
expect_status 2; expect_stderr "unknown option '--fast'"
finish

begin "missing file"
wl check nowhere.w
expect_status 1; expect_stderr "can't open 'nowhere.w'"
finish

begin "--version"
wl --version
expect_status 0; expect_stdout "wlang 0."
finish

begin "--help"
wl --help
expect_status 0; expect_stdout "Usage:"
finish

# ==================== make install ====================

begin "make install and uninstall"
make -s -C "$ROOT" install DESTDIR="$CASE" PREFIX=/usr/test >"$WORK/stdout" 2>"$WORK/stderr" || problem "make install failed"
expect_files usr/test/bin/wlang
"$CASE/usr/test/bin/wlang" run "$GOOD" >/dev/null; [ $? = 7 ] || problem "installed wlang didn't run the program"
make -s -C "$ROOT" uninstall DESTDIR="$CASE" PREFIX=/usr/test >/dev/null 2>&1 || problem "make uninstall failed"
expect_files
finish

echo
echo "$pass passed, $fail failed"
if [ "$fail" -gt 0 ]; then
    for f in "${failures[@]}"; do echo "  $f"; done
    exit 1
fi
