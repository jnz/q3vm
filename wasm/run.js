// run.js -- runs a .qvm image in q3vm compiled to WebAssembly.
// Usage: node run.js image.qvm command...
const fs = require("fs");
const createQ3vm = require("./node/vm.js");

createQ3vm().then((M) => {
  const image = fs.readFileSync(process.argv[2]);
  const ptr = M._alloc_bytes(image.length);
  M.HEAPU8.set(image, ptr);
  if (M._load(ptr, image.length) !== 0) {
    console.log("load failed");
    process.exit(2);
  }
  for (const arg of process.argv.slice(3)) {
    console.log("call", arg, "->", String(M._call64(parseInt(arg, 10))));
  }
});
