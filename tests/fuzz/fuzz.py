#!/usr/bin/env python3
"""Mutation fuzzer for wlang.

Takes the .w programs in tests/ and examples/ as seeds, mutates them (byte flips,
deleted / duplicated / swapped lines, spliced programs, random tokens, broken
indentation, deep nesting and long operator chains) and runs each result through `wlang build --emit-c` with a C compiler
that does nothing, so the lexer, parser, checker and code generator all run.

wlang must answer every input with exit code 0 (built) or 1 (W errors). Anything
else -- a sanitizer report (forced to exit 99), a signal, a hang -- is a bug.
Failing inputs are saved to the output directory so they can be replayed.

Usage: tests/fuzz/fuzz.py [--wlang PATH] [--runs N] [--seed S] [--out DIR]
"""

import argparse
import os
import random
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

# pieces of W that make mutated programs get further than random bytes would
TOKENS = [
    "fun", "dec", "ret", "log", "say", "when", "lest", "elf", "nor", "else",
    "loop", "while", "until", "forever", "skip", "stop", "w", "use",
    "num", "real", "chr", "str", "bool", "zil", "true", "false",
    "and", "or", "not", "is", "eq", "ne", "lt", "gt", "le", "ge",
    "+", "-", "*", "/", "%", "mod", "==", "!=", "<", ">", "<=", ">=", "!", "&&", "||",
    "(", ")", "[", "]", "{", "}", ",", ":", ":=", "=", ";",
    "0", "1", "-1", "2147483647", "99999999999999999999", "3.14", "1e309",
    "'a'", "'\\n'", "''", '"text"', '""', '"unterminated',
    "x", "y", "w()", "f(1, 2)", "--", "---", "note", "\t", "\n", "    ", "\r\n",
    "pod", ".", ".x", "Vec(x: 1, y: 2)", "b.pos.x", "x: 1",
]


def seeds():
    files = sorted(ROOT.glob("tests/**/*.w")) + sorted(ROOT.glob("examples/*.w"))
    return [f.read_bytes() for f in files]


def deep_chunk(rng: random.Random) -> list:
    """Nesting around the parser's limits (150 expression / 100 statement levels) and far past them."""
    k = rng.choice([rng.randint(1, 200), rng.randint(140, 160), rng.randint(90, 110), rng.randint(1000, 20000)])
    shape = rng.randrange(7)
    if shape == 5:
        # a long elf chain: not nested in W, but nested in the C (else-if) and in free_ast
        k = rng.choice([rng.randint(1, 50), rng.randint(990, 1010), rng.randint(5000, 50000)])
        chunk = [b"    say true", b"        skip"]
        for _ in range(k):
            chunk += [b"    elf false", b"        skip"]
        return chunk
    if shape == 0:
        return [b"    dec d: num = " + b"(" * k + b"1" + b")" * rng.choice([k, k - 1, k + 1])]
    if shape == 1:
        return [b"    dec d: bool = " + b"not " * k + b"true"]
    if shape == 6:
        return [b"    dec d: num = " + b"- " * k + b"1"]
    if shape == 2:
        return [b"    dec d: num = " + b" + ".join([b"1"] * k)]
    if shape == 3:
        return [b"    " + b"say true, " * k + b"skip"]
    k = min(k, 400)    # nested blocks: one line per level, and the file grows with k squared
    return [b"    " * (i + 1) + b"say true" for i in range(k)] + [b"    " * (k + 1) + b"skip"]


def mutate(data: bytes, corpus: list, rng: random.Random) -> bytes:
    lines = data.split(b"\n")
    if rng.randrange(10) == 0:                      # sometimes: a function with deep nesting
        lines += [b"", b"fun deep()"] + deep_chunk(rng)
    for _ in range(rng.randint(1, 4)):
        kind = rng.randrange(8)
        if kind == 0 and data:                      # flip a byte
            buf = bytearray(b"\n".join(lines))
            buf[rng.randrange(len(buf))] = rng.randrange(256)
            lines = bytes(buf).split(b"\n")
        elif kind == 1 and len(lines) > 1:          # delete a line
            del lines[rng.randrange(len(lines))]
        elif kind == 2 and lines:                   # duplicate a line
            i = rng.randrange(len(lines))
            lines.insert(rng.randrange(len(lines) + 1), lines[i])
        elif kind == 3 and len(lines) > 1:          # swap two lines
            i, j = rng.randrange(len(lines)), rng.randrange(len(lines))
            lines[i], lines[j] = lines[j], lines[i]
        elif kind == 4 and lines:                   # insert a token mid-line
            i = rng.randrange(len(lines))
            pos = rng.randint(0, len(lines[i]))
            tok = rng.choice(TOKENS).encode()
            lines[i] = lines[i][:pos] + b" " + tok + b" " + lines[i][pos:]
        elif kind == 5 and lines:                   # change a line's indentation
            i = rng.randrange(len(lines))
            lines[i] = b" " * rng.choice([0, 1, 2, 3, 4, 8, 12]) + lines[i].lstrip(b" \t")
        elif kind == 6:                             # splice in part of another program
            other = rng.choice(corpus).split(b"\n")
            a = rng.randrange(len(other))
            chunk = other[a:a + rng.randint(1, 6)]
            at = rng.randint(0, len(lines))
            lines[at:at] = chunk
        elif kind == 7 and lines:                   # cut a line short
            i = rng.randrange(len(lines))
            lines[i] = lines[i][:rng.randint(0, len(lines[i]))]
    return b"\n".join(lines)


def sanitizer_env() -> dict:
    """The environment for wlang: sanitizer reports must exit 99, never 1 like a W error.
    Settings already in ASAN_OPTIONS / UBSAN_OPTIONS (e.g. detect_leaks from make) are kept."""
    env = dict(os.environ)
    for var, defaults in (("ASAN_OPTIONS", "exitcode=99"), ("UBSAN_OPTIONS", "halt_on_error=1:exitcode=99")):
        given = env.get(var, "")
        missing = [d for d in defaults.split(":") if d.split("=")[0] + "=" not in given]
        env[var] = ":".join(filter(None, [given] + missing))
    return env


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--wlang", default=os.environ.get("WLANG", str(ROOT / "build/wlang")))
    parser.add_argument("--runs", type=int, default=2000)
    parser.add_argument("--seed", type=int, default=None, help="random seed (default: random, printed)")
    parser.add_argument("--out", default=str(ROOT / "build/fuzz-failures"))
    parser.add_argument("--timeout", type=float, default=10.0, help="seconds per input")
    args = parser.parse_args()

    seed = args.seed if args.seed is not None else random.randrange(2**32)
    rng = random.Random(seed)
    corpus = seeds()
    print(f"fuzz: {args.runs} runs, seed {seed}, {len(corpus)} seed programs, wlang {args.wlang}")

    failures = 0
    built = rejected = 0
    env = sanitizer_env()
    with tempfile.TemporaryDirectory() as work:
        src = Path(work) / "input.w"
        for run in range(args.runs):
            data = mutate(rng.choice(corpus), corpus, rng)
            src.write_bytes(data)
            cmd = [args.wlang, "build", str(src), "-o", str(Path(work) / "prog"),
                   "--emit-c", str(Path(work) / "prog.c"), "--cc", "true"]
            try:
                result = subprocess.run(cmd, capture_output=True, timeout=args.timeout, env=env)
                built += result.returncode == 0
                rejected += result.returncode == 1
                problem = None if result.returncode in (0, 1) else f"exit code {result.returncode}"
                detail = result.stderr.decode(errors="replace")
            except subprocess.TimeoutExpired:
                problem, detail = f"no answer after {args.timeout}s", ""

            if problem:
                failures += 1
                out = Path(args.out)
                out.mkdir(parents=True, exist_ok=True)
                saved = out / f"seed{seed}-run{run}.w"
                saved.write_bytes(data)
                print(f"FAIL  run {run}: {problem} (input saved to {saved})")
                print("      " + "\n      ".join(detail.strip().splitlines()[-15:]))
                if failures >= 5:
                    print("fuzz: stopping after 5 failures")
                    break

    print(f"fuzz: {built} built, {rejected} rejected with W errors, {failures} failure(s)")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
