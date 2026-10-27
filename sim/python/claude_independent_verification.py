#!/usr/bin/env python3
"""
Independent check of Rule 30 left-flank diagonals (single black cell).

Written by Claude (Anthropic AI model) in a chat session on 2026-10-28.
All results are finite-window measurements, not proofs.

Convention
  Rule 30: new[x] = old[x-1] XOR (old[x] OR old[x+1]); one black cell at x = 0, t = 0.
  Diagonal D_j(t) = cell at x = j - t at step t  (the set x + t = j).

Exact strip reduction
  D_j(t+1) = D_{j-2}(t) XOR (D_{j-1}(t) OR D_j(t)).
  D_j depends only on j-2, j-1, j, so truncating the strip at j = K is exact.
  One time step is three shifts on a Python integer whose bit j is D_j(t).

Usage
  python3 claude_independent_verification.py selftest
  python3 claude_independent_verification.py scan   --K 500000 --T0 750000 --T1 770000
  python3 claude_independent_verification.py words  --j 100000 150000 200000 250000 --T1 400000
  python3 claude_independent_verification.py parity --z 2 7 28 399 53207 58286 87866 --T 125000
  python3 claude_independent_verification.py hash "<exact string>"

Reference output from the original session (same parameters as above)
  scan:   zero diagonals = [2, 7, 28, 399, 53207, 58286, 87866]
  words:  D_100000 / D_150000 / D_200000 / D_250000 have period 32 and weights 12 / 12 / 16 / 16
  parity: C(z) = D_{z+1}(t) XOR (XOR over s<t of D_{z-1}(s)), for t after D_z's last 1, equals
          1, 0, 0, 0, 0, 1, 0 for z = 2, 7, 28, 399, 53207, 58286, 87866

Caveats
  - "Zero diagonal" here means: no 1s anywhere in the observation window [T0, T1].
    A diagonal whose last 1 falls after T0 would be missed (observed last-1 times are
    about 1.34 * j), and zeros over a finite window do not prove zeros forever.
  - Periods are measured over the last --window steps; they assume the transient is over.
"""
import argparse
import hashlib
import sys
import time


def make_stepper(K):
    """Return a function advancing the strip (bits 0..K) by one time step."""
    mask = (1 << (K + 1)) - 1

    def step(row):
        return ((row << 2) ^ ((row << 1) | row)) & mask

    return step


def brute_force(N):
    """Plain 2D Rule 30 for N steps; returns (history, index of x = 0)."""
    W = 2 * N + 5
    c = N + 2
    row = [0] * W
    row[c] = 1
    hist = [row[:]]
    for _ in range(N):
        new = [0] * W
        for x in range(1, W - 1):
            new[x] = row[x - 1] ^ (row[x] | row[x + 1])
        row = new
        hist.append(row[:])
    return hist, c


def cmd_selftest(args):
    N = 300
    hist, c = brute_force(N)
    step = make_stepper(2 * N)
    row = 1
    for t in range(1, N + 1):
        row = step(row)
        for j in range(0, 2 * t + 1):
            assert ((row >> j) & 1) == hist[t][c + j - t], (t, j)
    print(f"selftest OK: strip simulation == brute-force 2D Rule 30 for t <= {N}")


def cmd_scan(args):
    K, T0, T1 = args.K, args.T0, args.T1
    if T0 < K // 2 + 1:
        sys.exit("need T0 >= K/2 so every diagonal j <= K is inside the light cone")
    step = make_stepper(K)
    start = time.time()
    row, acc = 1, 0
    for t in range(1, T1 + 1):
        row = step(row)
        if t >= T0:
            acc |= row
    bits = bin(acc)[2:].zfill(K + 1)[::-1]  # bits[j] == '1' iff D_j had a 1 in the window
    zeros = [j for j, b in enumerate(bits) if b == "0"]
    print(f"diagonals j <= {K} with no 1s in window t = [{T0}, {T1}]: {zeros}")
    print(f"({time.time() - start:.1f} s)")


def minimal_period(seq, max_p=2048):
    n = len(seq)
    for p in range(1, max_p + 1):
        if all(seq[i] == seq[i + p] for i in range(n - p)):
            return p
    return None


def cmd_words(args):
    js = sorted(args.j)
    K, T1, W = max(js) + 1, args.T1, args.window
    if T1 - W < 1.4 * max(js):
        print("warning: window may start before the transient ends (last-1 times are ~1.34*j)")
    step = make_stepper(K)
    seqs = {j: [] for j in js}
    row = 1
    for t in range(1, T1 + 1):
        row = step(row)
        if t > T1 - W:
            for j in js:
                seqs[j].append((row >> j) & 1)
    for j in js:
        p = minimal_period(seqs[j])
        if p is None:
            print(f"D_{j}: no period <= 2048 found in the last {W} steps")
            continue
        word = seqs[j][-p:]
        line = f"D_{j}: period = {p}, weight = {sum(word)}"
        if p <= 64:
            # time-ordered bits, bit i of the integer = i-th step; minimum over rotations
            vals = []
            for r in range(p):
                w = word[r:] + word[:r]
                vals.append(sum(b << i for i, b in enumerate(w)))
            line += f", canonical word = 0x{min(vals):0{(p + 3) // 4}x} (min over rotations, LSB = earliest step)"
        print(line)


def cmd_parity(args):
    zs, T = args.z, args.T
    K = max(zs) + 1
    step = make_stepper(K)
    acc = {z: 0 for z in zs}
    series = {z: [] for z in zs}
    last1 = {z: -1 for z in zs}
    row = 1
    for t in range(0, T + 1):
        for z in zs:
            if (row >> z) & 1:
                last1[z] = t
            series[z].append(((row >> (z + 1)) & 1) ^ acc[z])
            acc[z] ^= (row >> (z - 1)) & 1
        row = step(row)
    for z in zs:
        L = last1[z]
        tail = sorted(set(series[z][L + 1:]))
        note = "" if len(tail) == 1 else "  <-- not constant or T too small"
        print(f"z = {z}: last 1 of D_z at t = {L}; C(z) over t > {L} takes values {tail}{note}")


def cmd_hash(args):
    print(hashlib.sha256(args.text.encode("utf-8")).hexdigest())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    sub.add_parser("selftest").set_defaults(func=cmd_selftest)

    p = sub.add_parser("scan")
    p.add_argument("--K", type=int, default=500000)
    p.add_argument("--T0", type=int, default=750000)
    p.add_argument("--T1", type=int, default=770000)
    p.set_defaults(func=cmd_scan)

    p = sub.add_parser("words")
    p.add_argument("--j", type=int, nargs="+", default=[100000, 150000, 200000, 250000])
    p.add_argument("--T1", type=int, default=400000)
    p.add_argument("--window", type=int, default=8192)
    p.set_defaults(func=cmd_words)

    p = sub.add_parser("parity")
    p.add_argument("--z", type=int, nargs="+", default=[2, 7, 28, 399, 53207, 58286, 87866])
    p.add_argument("--T", type=int, default=125000)
    p.set_defaults(func=cmd_parity)

    p = sub.add_parser("hash")
    p.add_argument("text")
    p.set_defaults(func=cmd_hash)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
