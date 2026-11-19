#!/usr/bin/env python3
"""vmpc.py -- names the function at a VM program counter, for VM_CHECKED reports.

Usage: vmpc.py image.map pc [pc...]

The map is written by q3asm -m. Its addresses are instruction indexes, the same
values as the program counter in a check report.
"""

import bisect
import sys


def load(path):
    entries = []
    with open(path) as f:
        for line in f:
            parts = line.split()
            if len(parts) == 3 and parts[0] == "0" and not parts[1].startswith("fffff"):
                entries.append((int(parts[1], 16), parts[2]))
    entries.sort()
    return entries


def name_at(entries, pc):
    starts = [start for start, _ in entries]
    i = bisect.bisect_right(starts, pc) - 1
    if i < 0:
        return "?"
    start, name = entries[i]
    return "%s+%d" % (name, pc - start)


def main(argv):
    if len(argv) < 3:
        print(__doc__)
        return 2
    entries = load(argv[1])
    for pc in argv[2:]:
        print("pc %s: %s" % (pc, name_at(entries, int(pc))))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
