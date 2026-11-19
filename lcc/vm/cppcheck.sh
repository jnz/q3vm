#!/bin/bash
# cppcheck.sh -- compares the preprocessor running in the VM (q3cpp.qvm) with the
# native one. Each C file is preprocessed both ways with the same arguments, the
# outputs must be the same, byte for byte, and the exit codes must match.
#
#   ./cppcheck.sh file.c...
#
# Run 'make' first so that q3cpp.qvm and vmrun64 exist.

set -u

HERE=$(cd "$(dirname "$0")" && pwd)
NATIVE="$HERE/../../bin/linux/q3cpp"
VMRUN="$HERE/vmrun64"
QVM="$HERE/q3cpp.qvm"
OUT="$HERE/build/cppcheck"
DEFINES="-D__STDC__=1 -D__STRICT_ANSI__ -D__signed__=signed -DQ3_VM -D__LCC__ -DLCC_WIDE32"
mkdir -p "$OUT"

fail=0
count=0
for f in "$@"; do
    count=$((count + 1))
    name=$(echo "$f" | tr '/' '_')
    # the same arguments both ways, the include directory is absolute
    "$NATIVE" $DEFINES -I"$HERE/include" "$f" "$OUT/$name.native" >/dev/null 2>&1
    native_status=$?
    "$VMRUN" "$QVM" $DEFINES -I"$HERE/include" "$f" "$OUT/$name.vm" >/dev/null 2>&1
    vm_status=$?
    if [ "$native_status" != "$vm_status" ] || ! cmp -s "$OUT/$name.native" "$OUT/$name.vm"; then
        echo "FAIL $f (native $native_status, vm $vm_status)"
        fail=$((fail + 1))
    else
        echo "ok   $f"
    fi
done

echo "$count files, $fail differ"
[ "$fail" = 0 ]
