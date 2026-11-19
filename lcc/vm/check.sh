#!/bin/bash
# check.sh -- compares the compiler running in the VM with the native compiler.
#
# For each C file, the file is preprocessed with q3cpp, then compiled to LCC
# bytecode by the native rcc and by rcc.qvm in vmrcc. The outputs must be the
# same, byte for byte, and the exit codes must match.
#
#   ./check.sh [-I dir] file.c...
#
# -I is relative to the directory of each file, as for the lcc driver. Run
# 'make' first so that rcc.qvm and vmrcc exist.

set -u

HERE=$(cd "$(dirname "$0")" && pwd)
export PATH="$HERE/../../bin/linux:$PATH"
QVM="$HERE/rcc.qvm"
VMRCC="${VMRCC:-$HERE/vmrcc}" # VMRCC=$HERE/vmrcc_checked for the checked VM
DEFINES="-D__STDC__=1 -D__STRICT_ANSI__ -D__signed__=signed -DQ3_VM -D__LCC__"
# the compiler's own sources use the 32-bit Wide type for a 32-bit target
if [ "${TARGET:-bytecode}" = "bytecode" ]; then
    DEFINES="$DEFINES -DLCC_WIDE32"
fi

INCLUDE=""
if [ "${1:-}" = "-I" ]; then
    INCLUDE="$2"
    shift 2
fi

TMP=$(mktemp -d "${TMPDIR:-/tmp}/rcccheck.XXXXXX") || exit 2
trap 'rm -rf "$TMP"' EXIT

pass=0
fail=0
for file in "$@"; do
    abs=$(cd "$(dirname "$file")" && pwd)/$(basename "$file")
    name=$(basename "$file" .c)
    dir=$(dirname "$abs")
    base=$(basename "$abs")
    if ! (cd "$dir" && q3cpp $DEFINES ${INCLUDE:+-I"$INCLUDE"} "$base" > "$TMP/$name.i" 2> "$TMP/$name.cpp.err"); then
        echo "SKIP $file: preprocessing failed"
        continue
    fi

    q3rcc -target=${TARGET:-bytecode} "$TMP/$name.i" "$TMP/$name.native.asm" 2> "$TMP/$name.native.err"
    native_status=$?
    "$VMRCC" "$QVM" -target=${TARGET:-bytecode} "$TMP/$name.i" "$TMP/$name.vm.asm" 2> "$TMP/$name.vm.err"
    vm_status=$?

    if [ "$native_status" -ne 0 ] || [ "$vm_status" -ne 0 ]; then
        echo "FAIL $file: compile errors, native $native_status, vm $vm_status"
        fail=$((fail + 1))
    elif [ "$native_status" -ne "$vm_status" ]; then
        echo "FAIL $file: exit status native $native_status, vm $vm_status"
        fail=$((fail + 1))
    elif ! cmp -s "$TMP/$name.native.asm" "$TMP/$name.vm.asm"; then
        echo "FAIL $file: output differs"
        fail=$((fail + 1))
    else
        echo "ok   $file ($(wc -l < "$TMP/$name.native.asm") lines)"
        pass=$((pass + 1))
    fi
done

echo "passed $pass, failed $fail"
[ "$fail" -eq 0 ]
