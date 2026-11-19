// ide.js -- the core of the mini IDE: runs C programs in q3vm, in the browser or in Node.
//
// The toolchain (driver, compiler, assembler, preprocessor, runtime library and
// headers) is loaded from tools/manifest.json, made by wasm/Makefile. The user's files
// are put in the VM's file system, flat, by base name, as lcc-run does. Then the
// driver (driver.qvm) compiles and runs the program, the way lcc-run does natively.
//
// Use: const ide = await Q3IDE.start(createQ3vm, fetchBytes, { print, printErr });
//      const status = ide.run("hello.c", files, ["alpha", "beta"]);
//      const asm = ide.compile("hello.c", files);   // the assembly, after the compiler

(function (root) {
  "use strict";

  const encoder = new TextEncoder();
  const decoder = new TextDecoder();

  // the defines of the preprocessor, as lcc-run passes them to it
  const DEFINES = ["-D__STDC__=1", "-D__STRICT_ANSI__", "-D__signed__=signed", "-DQ3_VM", "-D__LCC__"];

  async function start(createVM, fetchBytes, output) {
    const M = await createVM({
      print: (text) => output.print(text),
      printErr: (text) => output.printErr(text),
    });
    M._vm_init();

    // allocates a copy of bytes in the module memory, freed with free()
    const alloc = (bytes) => {
      const p = M._alloc_bytes(bytes.length + 1);
      M.HEAPU8.set(bytes, p);
      M.HEAPU8[p + bytes.length] = 0;
      return p;
    };

    const putFile = (name, bytes) => {
      const n = alloc(encoder.encode(name));
      const d = alloc(bytes);
      M._put_file(n, d, bytes.length);
      M._free_bytes(n);
      M._free_bytes(d);
    };

    const getFile = (name) => {
      const n = alloc(encoder.encode(name));
      const size = M._file_size(n);
      if (size < 0) {
        M._free_bytes(n);
        return null;
      }
      const buf = M._alloc_bytes(size + 1);
      M._get_file(n, buf, size);
      const out = M.HEAPU8.slice(buf, buf + size);
      M._free_bytes(buf);
      M._free_bytes(n);
      return out;
    };

    // runs main of an image with argv, returns its result
    const runImage = (image, argv) => {
      const img = alloc(image);
      const args = alloc(concatStrings(argv));
      const status = M._run(img, image.length, argv.length, args);
      M._free_bytes(img);
      M._free_bytes(args);
      return status;
    };

    // the toolchain: the files of the payload of lcc-run, in the root of the VM
    const manifest = JSON.parse(decoder.decode(await fetchBytes("tools/manifest.json")));
    for (const entry of manifest.tools) {
      putFile(entry.dst, await fetchBytes("tools/" + entry.src));
    }
    const driver = await fetchBytes("tools/driver.qvm");

    // puts the user's files in the root of the VM, by base name
    const putUserFiles = (files) => {
      for (const [name, text] of Object.entries(files)) {
        putFile(baseName(name), encoder.encode(text));
      }
    };

    const ide = {
      // the example programs: { name, src } where src is under tools/
      examples: manifest.examples,
      // the text of an example, to put in the editor
      async example(entry) {
        return decoder.decode(await fetchBytes("tools/" + entry.src));
      },
      // compiles and runs a file, with its arguments; returns the status of the program
      // (1 when the compiler or the linker fails, as in lcc-run)
      run(name, files, args) {
        putUserFiles(files);
        return runImage(driver, ["driver", baseName(name), ...args]);
      },
      // runs the compiler only and returns the assembly, or null when it fails
      compile(name, files) {
        putUserFiles(files);
        const status = runImage(driver, ["driver", "-S", baseName(name)]);
        if (status !== 0) {
          return null;
        }
        const base = baseName(name).replace(/\.c$/, "");
        const asm = getFile(base + ".asm");
        return asm ? decoder.decode(asm) : null;
      },
    };
    return ide;
  }

  function baseName(path) {
    const slash = path.lastIndexOf("/");
    return slash < 0 ? path : path.slice(slash + 1);
  }

  // the NUL terminated strings one after the other, as the run function reads them
  function concatStrings(strings) {
    const parts = strings.map((s) => encoder.encode(s + "\0"));
    const total = parts.reduce((n, p) => n + p.length, 0);
    const out = new Uint8Array(total);
    let at = 0;
    for (const p of parts) {
      out.set(p, at);
      at += p.length;
    }
    return out;
  }

  const Q3IDE = { start, baseName };
  if (typeof module !== "undefined" && module.exports) {
    module.exports = Q3IDE;
  } else {
    root.Q3IDE = Q3IDE;
  }
})(typeof globalThis !== "undefined" ? globalThis : this);
