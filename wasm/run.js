// run.js -- compiles and runs a C file with the toolchain in q3vm, in WebAssembly.
// It does what lcc-run does: the files go in the file system of the VM, then the
// driver (driver.qvm, from lcc/vm) compiles and runs the source. Files are in memory.
//
// Usage: node run.js <lcc/vm directory> file.c [arguments...]
// Build first: make node (here), and make driver.qvm in lcc/vm.
const fs = require("fs");
const path = require("path");
const createQ3vm = require("./node/vm.js");

const toolDir = path.resolve(process.argv[2]);
const programArgs = process.argv.slice(3);

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

  // runs main of an image with argv, returns its result
  const runImage = (image, argv) => {
    const img = alloc(image);
    const args = alloc(Buffer.concat(argv.map((a) => Buffer.from(a + "\0"))));
    const status = M._run(img, image.length, argv.length, args);
    free(img);
    free(args);
    return status;
  };

  // the files of the toolchain, as the payload of lcc-run has them, in the root
  const tool = (name) => fs.readFileSync(path.join(toolDir, name));
  for (const name of ["driver.qvm", "q3cpp.qvm", "q3asm.qvm", "rcc.qvm"]) {
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
  // the source file and the arguments that are files, by their base names
  for (const arg of programArgs) {
    if (fs.existsSync(arg) && fs.statSync(arg).isFile()) {
      putFile(path.basename(arg), fs.readFileSync(arg));
    }
  }

  const status = runImage(tool("driver.qvm"), ["driver", ...programArgs]);
  console.log("exit " + status);
});
