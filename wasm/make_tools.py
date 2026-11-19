#!/usr/bin/env python3 -I
# make_tools.py -- copies the toolchain of lcc/vm, the files of lcc-run's payload, into
# web/tools, with a manifest that the mini IDE reads (web/ide.js).
#
#   python3 -I make_tools.py ../lcc/vm web/tools
#
# Run 'make' in lcc/vm first (the default, 32-bit build), so that the files exist.
import json
import os
import shutil
import sys

src, dst = sys.argv[1], sys.argv[2]

tools = [
    ("driver.qvm", "driver.qvm"),
    ("q3cpp.qvm", "q3cpp.qvm"),
    ("q3asm.qvm", "q3asm.qvm"),
    ("rcc.qvm", "rcc.qvm"),
    ("build/vmmain.asm", "lib/vmmain.asm"),
    ("build/vmlib.asm", "lib/vmlib.asm"),
    ("build/bg_lib.asm", "lib/bg_lib.asm"),
    ("vm_syscalls.asm", "lib/vm_syscalls.asm"),
]
headers = sorted(f for f in os.listdir(os.path.join(src, "include")) if f.endswith(".h"))
examples = ["hello.c", "mandelbrot.c"]

entries = []


def copy(rel_src, rel_dst):
    source = os.path.join(src, rel_src)
    if not os.path.exists(source):
        sys.exit("missing %s, run make in %s first" % (source, src))
    target = os.path.join(dst, rel_dst)
    os.makedirs(os.path.dirname(target), exist_ok=True)
    shutil.copyfile(source, target)
    return rel_dst


for rel_src, rel_dst in tools:
    entries.append({"src": copy(rel_src, rel_dst), "dst": rel_dst})
for h in headers:
    entries.append({"src": copy(os.path.join("include", h), os.path.join("include", h)),
                    "dst": os.path.join("include", h)})

example_entries = []
for e in examples:
    rel = os.path.join("examples", e)
    copy(rel, rel)
    example_entries.append({"name": e, "src": rel})

manifest = {"tools": entries, "examples": example_entries}
os.makedirs(dst, exist_ok=True)
with open(os.path.join(dst, "manifest.json"), "w") as f:
    json.dump(manifest, f, indent=1)
print("tools: %d files, %d examples" % (len(entries), len(example_entries)))
