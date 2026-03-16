To build LCC as an [APE executable](https://justine.lol/ape.html) that can be run on pretty much all the OS's, follow these steps:


Download cosmocc toolchain and unzip it in cosmocc dir:

```sh
cd cosmocc
wget https://cosmo.zip/pub/cosmocc/cosmocc.zip
unzip cosmocc.zip
```

Then compile the "build script" and run it:
> Sidenote: You can actually debug the "build script" if you pass `-ggdb3` to the command
```sh
cc -o knob knob.c
./knob
```

You should then have in the build directory:
- q3rcc.com
- q3cpp.com
- lcc.com

These binaries can run on Windows,Mac OS X, Linux, FreeBSD, OpenBSD, etc.