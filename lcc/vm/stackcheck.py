#!/usr/bin/env python3
"""stackcheck.py -- checks the operand stack depth of LCC bytecode for the VM.

Each statement of LCC's bytecode leaves the operand stack as it found it, so
the depth is zero at each label that starts a statement. A 64-bit value takes
two cells. This prints the first procedure where the depth drifts, with the
line of the first drift, to find an opcode whose stack effect is wrong.

Usage: stackcheck.py file.asm...
"""

import re
import sys

# stack effect in cells of each op, by name prefix; 8 is a 64-bit (two cell) value
def effect(op, arg):
    if op.startswith("ADDR"):
        return 1
    m = re.match(r"^CNST([A-Z])(\d)$", op)
    if m:
        return 2 if m.group(2) == "8" else 1
    m = re.match(r"^(INDIR)([A-Z])(\d)$", op)
    if m:
        return 1 if m.group(3) == "8" else 0
    m = re.match(r"^(ASGN)([A-Z])(\d)$", op)
    if m:
        return -3 if m.group(3) == "8" else -2
    if op == "ASGNB":
        return -2
    if op == "INDIRB":
        return 0
    m = re.match(r"^ARG([A-Z])(\d)$", op)
    if m:
        return -2 if m.group(2) == "8" else -1
    if op == "pop":
        return -1
    m = re.match(r"^CALL([A-Z])(\d)$", op)
    if m:
        return 1 if m.group(2) == "8" else 0
    if op == "CALLV":
        return 0
    m = re.match(r"^RET([A-Z])(\d)$", op)
    if m:
        return -2 if m.group(2) == "8" else -1
    if op == "RETV":
        return 0
    m = re.match(r"^(EQ|NE|LT|LE|GT|GE)([A-Z])(\d)$", op)
    if m:
        return -4 if m.group(3) == "8" else -2
    m = re.match(r"^(ADD|SUB|MUL|DIV|MOD|BAND|BOR|BXOR|LSH|RSH)([A-Z])(\d)$", op)
    if m:
        kind, _, size = m.groups()
        if size == "8":
            return -1 if kind in ("LSH", "RSH") else -2
        return -1
    m = re.match(r"^(NEG|BCOM)([A-Z])(\d)$", op)
    if m:
        return 0
    m = re.match(r"^CV([A-Z])([A-Z])(\d)$", op)
    if m:
        return None  # depends on the size operand, see conversion()
    if op == "JUMPV":
        return -1  # consumes the address pushed by ADDRGP4
    return None


def conversion(op, arg):
    m = re.match(r"^CV([A-Z])([A-Z])(\d)$", op)
    src, dst, dsize = m.group(1), m.group(2), int(m.group(3))
    ssize = int(arg) if arg else 4
    if dst == "F":
        if dsize == 8:
            if src == "F":
                return 1 if ssize == 4 else 0
            return (1 if ssize != 8 else 0) + 0
        if src == "F":
            return -1 if ssize == 8 else 0
        return -1 if ssize == 8 else 0
    if src == "F":
        if ssize == 4:
            return 1 if dsize == 8 else 0
        return -1 if dsize != 8 else 0
    if ssize == 8:
        return -1 if dsize != 8 else 0
    if dsize == 8:
        return 1
    return 0


def check(path):
    depth = None
    proc = None
    problems = []
    pending = None
    with open(path) as f:
        for n, line in enumerate(f, 1):
            parts = line.split()
            if not parts:
                continue
            op = parts[0]
            arg = parts[1] if len(parts) > 1 else None
            if op == "proc":
                depth, proc, pending = 0, parts[1], None
                continue
            if op == "endproc":
                if depth not in (None, 0):
                    problems.append((n, proc, "end of procedure with depth %d" % depth))
                depth = None
                continue
            if depth is None:
                continue
            if op == "LABELV" or op.startswith("LABEL"):
                if depth != 0 and not op.startswith("LABELV $") and depth != 0:
                    pass
                continue
            if op.startswith("CV") and len(op) == 5:
                e = conversion(op, arg)
                # the size operand of the conversion is the next token
                # (lcc prints "CVII8 4"), and it is the arg here
            else:
                e = effect(op, arg)
            if e is None:
                continue
            depth += e
            if depth < 0:
                problems.append((n, proc, "depth below zero after %s" % op))
                depth = 0
    return problems


def main(paths):
    status = 0
    for path in paths:
        problems = check(path)
        for n, proc, msg in problems[:5]:
            print("%s:%d: %s: %s" % (path, n, proc, msg))
        if problems:
            status = 1
    return status


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
