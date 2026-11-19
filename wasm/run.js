// run.js -- compiles and runs a C file with the toolchain in q3vm, in WebAssembly.
// The steps are those of lcc/vm/lcc-run.c, and the files are in memory.
//
// Usage: node run.js <lcc/vm directory> file.c [arguments...]
// Build first: make node (here), and make WIDTH=64 in lcc/vm.
const fs = require("fs");
const path = require("path");
const createQ3vm = require("./node/vm.js");

const toolDir = path.resolve(process.argv[2]);
const source = process.argv[3];
const programArgs = process.argv.slice(4);
const base = path.basename(source, ".c");

const defines = [
  "-D__STDC__=1", "-D__STRICT_ANSI__", "-D__signed__=signed", "-DQ3_VM", "-D__LCC__",
];

createQ3vm().then((M) => {
  M._vm_init();

  // allocates a copy of bytes in the module memory, free it with free(p)
  const alloc = (bytes) => {
    const p = M._alloc_bytes(bytes.length + 1);
    M.HEAPU8.set(bytes, p);
    M.HEAPU8[p + bytes.length] = 0;
    return p;
  };
  const free = (p) => M._free_bytes(p);

  const putFile = (name, data) => {
    const n = alloc(Buffer.from(name));
    const d = alloc(data);
    M._put_file(n, d, data.length);
    free(n);
    free(d);
  };

  const getFile = (name) => {
    const n = alloc(Buffer.from(name));
    const size = M._file_size(n);
    if (size < 0) {
      free(n);
      return null;
    }
    const buf = M._alloc_bytes(size + 1);
    M._get_file(n, buf, size);
    const out = Buffer.from(M.HEAPU8.slice(buf, buf + size));
    free(buf);
    free(n);
    return out;
  };

  // runs main of the image file with argv, returns its result
  const runImage = (name, argv) => {
    const image = getFile(name);
    if (!image) {
      console.log("no file " + name);
      process.exit(1);
    }
    const img = alloc(image);
    const args = alloc(Buffer.concat(argv.map((a) => Buffer.from(a + "\0"))));
    const status = M._run(img, image.length, argv.length, args);
    free(img);
    free(args);
    return status;
  };

  // the files of the toolchain, as lcc-run's payload has them
  const tool = (name) => fs.readFileSync(path.join(toolDir, name));
  for (const name of ["q3cpp.qvm", "q3asm.qvm", "rcc.qvm"]) {
    putFile(name, tool(name));
  }
  const libs = {
    "lib/vmmain.asm": "build/vmmain.asm",
    "lib/vmlib.asm": "build/vmlib.asm",
    "lib/bg_lib.asm": "build/bg_lib.asm",
    "lib/vm_syscalls.asm": "vm_syscalls.asm",
  };
  for (const [name, file] of Object.entries(libs)) {
    putFile(name, tool(file));
  }
  const includeDir = path.join(toolDir, "include");
  for (const h of fs.readdirSync(includeDir)) {
    if (h.endsWith(".h")) {
      putFile("include/" + h, fs.readFileSync(path.join(includeDir, h)));
    }
  }
  putFile(base + ".c", fs.readFileSync(source));

  const fail = (step, status) => {
    console.log(step + " failed with " + status);
    process.exit(1);
  };

  // 1. preprocess
  let status = runImage("q3cpp.qvm", [
    "q3cpp", ...defines, "-Iinclude", base + ".c", base + ".i",
  ]);
  if (status !== 0) fail("preprocessing", status);

  // 2. compile
  status = runImage("rcc.qvm", [
    "rcc", "-target=bytecode64", base + ".i", base + ".asm",
  ]);
  if (status !== 0) fail("compilation", status);

  // 3. link with the runtime library
  status = runImage("q3asm.qvm", [
    "q3asm", "-o", "prog", "lib/vmmain", "lib/vmlib", "lib/bg_lib", base, "lib/vm_syscalls",
  ]);
  if (status !== 0) fail("linking", status);

  // 4. run the program, its argv[0] is the name of the source
  status = runImage("prog.qvm", [base, ...programArgs]);
  console.log("exit " + status);
});
