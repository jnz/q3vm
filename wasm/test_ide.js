// test_ide.js -- checks the core of the mini IDE (web/ide.js) in Node, with the same
// module and the same toolchain files as the page. Run: make node ide && node test_ide.js
const assert = require("assert");
const fs = require("fs");
const path = require("path");
const createQ3vm = require("./node/vm.js");
const Q3IDE = require("./web/ide.js");

// the page fetches "tools/..." relative to web/, so the same paths are read here
const webDir = path.join(__dirname, "web");
const fetchBytes = async (rel) => new Uint8Array(fs.readFileSync(path.join(webDir, rel)));

// the module gives each line of output without its newline, as the console shows it
const text = (lines) => lines.map((l) => l + "\n").join("");

async function newIde() {
  const out = [];
  const err = [];
  const ide = await Q3IDE.start(createQ3vm, fetchBytes, {
    print: (text) => out.push(text),
    printErr: (text) => err.push(text),
  });
  return { ide, out, err };
}

async function main() {
  // 1. a program with arguments, from the examples
  {
    const { ide, out } = await newIde();
    const example = await ide.example(ide.examples[0]);
    const status = ide.run("hello.c", { "hello.c": example }, ["alpha", "beta"]);
    assert.strictEqual(status, 0, "hello.c exit status");
    assert.strictEqual(text(out), "argc 3 first alpha\nsum 30\n");
    console.log("ok   hello.c runs with its arguments");
  }

  // 2. the compiler alone gives the assembly of the program
  {
    const { ide } = await newIde();
    const asm = ide.compile("hello.c", { "hello.c": await ide.example(ide.examples[0]) });
    assert.ok(asm && asm.includes("proc main"), "assembly has main");
    console.log("ok   compile returns the assembly");
  }

  // 3. a compile error is reported, and the status is 1, as in lcc-run
  {
    const { ide, err } = await newIde();
    const status = ide.run("bad.c", { "bad.c": "int main(void) { return nothere; }\n" }, []);
    assert.strictEqual(status, 1, "bad.c exit status");
    assert.ok(text(err).includes("undeclared identifier"), "error text");
    console.log("ok   a compile error gives status 1 and the message");
  }

  // 4. a header next to the source, by base name, as the page puts the files
  {
    const { ide, out } = await newIde();
    const files = {
      "src/main.c": '#include "helper.h"\n#include <stdio.h>\nint main(void) { printf("%d\\n", twice(21)); return 0; }\n',
      "src/helper.h": "static int twice(int x) { return 2 * x; }\n",
    };
    const status = ide.run("src/main.c", files, []);
    assert.strictEqual(status, 0, "main.c exit status");
    assert.strictEqual(text(out), "42\n");
    console.log("ok   a user header is found by base name");
  }

  // 5. the same module runs more than one program
  {
    const { ide, out } = await newIde();
    const source = '#include <stdio.h>\nint main(void) { printf("again\\n"); return 7; }\n';
    assert.strictEqual(ide.run("a.c", { "a.c": source }, []), 7);
    assert.strictEqual(ide.run("a.c", { "a.c": source }, []), 7);
    assert.strictEqual(text(out), "again\nagain\n");
    console.log("ok   a second run in the same module");
  }
}

main().then(
  () => console.log("all IDE core checks passed"),
  (e) => {
    console.error("FAIL", e.message);
    process.exit(1);
  },
);
