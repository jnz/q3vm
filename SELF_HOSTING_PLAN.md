# Self-hosting LCC on q3vm: plan

Goal: LCC (the C compiler in `lcc/`) compiled to `.qvm` bytecode and run inside
q3vm. The VM-hosted compiler must compile LCC's own source and produce the same
`.asm` output as the native compiler.

Base branch: `int64-double-native` (64-bit integers and doubles are native).

## Status

- Stages 1, 3 and 4 work. The VM-hosted `rcc` (`lcc/vm/rcc.qvm`, run by
  `lcc/vm/vmrcc`) produces `.asm` identical to the native `rcc` for all 29 `.c`
  files in `lcc/src`, and for the q3 firmware sources. Run `make test` in
  `lcc/vm`.
- Stage 2 is not needed: the image and stack are within the existing limits.
- Stage 5 (`cpp` in the VM) is not started. The preprocessing is still native.
- The checked VM (`lcc/vm/vmrcc_checked`, built with `-DVM_CHECKED`) is the
  valgrind-like mode. It reports reads of uninitialized memory, misaligned or
  out-of-range accesses, division by zero, and operand stack depth errors at
  return. `vmpc.py` names the function for a program counter in a report.
  `make test` runs the whole source set through it too.
- Bugs found and fixed on the way (all on this branch):
  - q3asm: frames are multiples of 8 bytes, so 8-byte arguments are aligned in
    memory and the VM entry offset is too. Variadic `long long` and `double`
    arguments read wrong without this.
  - q3asm: initialized data of one label stays in one segment. A struct that
    mixed char and int fields was split and read back as garbage.
  - q3asm: 16-bit initialized data is accepted (it was an error).
  - lcc: a pointer index wider than a pointer (`unsigned long long`) is cut to
    pointer size before the add.
  - lcc: file-scope statics get a per-file label in the bytecode target, so
    modules of one program don't collide.
  - lcc: the bytecode target was declared big endian (`little_endian = 0`),
    so bit-fields were laid out and initialized in the wrong order.
  - VM: `vm_strtod` is a host call, so `double` literals convert exactly like
    the native compiler.

## Findings

Survey of `lcc/src` (about 12,500 lines):

1. **No I/O in the VM.** LCC reads source from `stdin` and files, writes `.asm`
   output with `fopen`/`fwrite`, and formats text with its own `fprint`,
   `stringf` and `%D`/`%U`/`%S` conversions. The VM has no libc; the host must
   provide file and stdio syscalls, and the bytecode needs a `vsprintf` that
   understands these formats.
2. **No heap.** LCC allocates from arenas via `malloc`-style calls. Options: a
   bump allocator in bytecode backed by a large `.bss`, or a host `malloc`
   syscall that returns addresses in the VM data segment.
3. **Limits.** `VM_MAX_BSS_LENGTH` is 10 MB, the image is capped at
   `VM_MAX_IMAGE_SIZE` (4 MB), and the stack is a fixed 64 KB
   (`VM_PROGRAM_STACK_SIZE`, hard-coded in q3asm). LCC's recursive parser may
   exceed the stack.
4. **`long` is 32-bit in the VM.** LCC keeps constant values in `long`
   (`Value.i`, `icon()`, `consttree`) and casts pointers to `unsigned long`.
   Compiled for the VM these become 32-bit, which truncates constants above 2^31.
5. **Driver spawns subprocesses.** `lcc` runs `cpp` and `rcc` as separate
   programs.
6. **Not blockers.** LCC uses no `setjmp`/`longjmp`. `exit` is the only error
   path.

## Stages

### Stage 1: run `rcc` in the VM

- Compile `rcc` (the compiler proper, fed preprocessed input) to `.qvm`.
- Add host syscalls for: read stdin, open/read/write/close files, exit, and
  time if needed.
- Add a bytecode `vsprintf` with LCC's `%D`, `%U`, `%S` formats.
- Add a bytecode allocator (bump allocator in `.bss`, or a host `malloc`
  syscall).
- Acceptance: for a set of small C programs, the VM-hosted `rcc` produces
  `.asm` byte-identical to the native `rcc`.

### Stage 2: raise limits

- Raise `VM_MAX_BSS_LENGTH` and `VM_MAX_IMAGE_SIZE` as needed.
- Decide the stack size. `VM_PROGRAM_STACK_SIZE` is in q3asm and the VM; it
  must change in both.
- Acceptance: stage 1 still passes on larger inputs.

### Stage 3: `long` becomes `long long`

- Convert LCC's uses of `long`/`unsigned long` that hold constant values or
  pointers to `long long`/`unsigned long long` or pointer-sized types.
  About 60 call sites, including `Value` in `c.h`, `icon()` in `lex.c`,
  `consttree`/`cnsttree`, and `defconst` in `bytecode.c`.
- Acceptance: native LCC built from the converted source produces the same
  output as before.

### Stage 4: self-host check

- Run the VM-hosted `rcc` over LCC's own source, with preprocessing done
  natively.
- Compare its `.asm` output with the native compiler's output, file by file.
  Bit-identical output is the pass condition.
- Bootstrap check: VM-hosted `rcc` compiles LCC, the result is run in the VM
  again, and the output is compared again.

### Stage 5 (optional): `cpp` in the VM

- Compile the preprocessor to `.qvm` as well, so the whole toolchain runs in the
  VM and the native preprocessing step goes away.

## Open decisions

- Whether `cpp` must run in the VM, or native preprocessing is acceptable.
- Host `malloc` syscall vs bytecode bump allocator.
- Stack size and memory budget for LCC running in the VM.

## Risks

- The VM interpreter is slow compared to native code. Compiling LCC's own source
  in the VM may take a long time. Measure stage 1 first.
- Any `long` the conversion misses silently truncates constants. Stage 3's
  output comparison is the check for this.
- Windows: `bin/win32` binaries are still the old toolchain, so the Windows
  build is not part of this plan.
