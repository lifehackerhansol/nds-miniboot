# miniboot

Minimal flashcart bootstrap for NDS consoles. Loads `BOOT.NDS` and
papers over various device-specific quirks, allowing running 100%
clean homebrew launchers on a variety of such devices.

Hold START while loading to enable debug output.

## Motivation

`.nds` files can be loaded essentially anywhere in RAM: in particular,
3.75MB out of the 4MB of main RAM can be used. As such, the easiest
way to load such a file is to put the bootstrap code outside main RAM.

As the fastest place to execute code from is ITCM, I started wondering
if one could create a bootstrap that operates entirely from ITCM (32K)
and DTCM (16K). That is not a lot of room - the DLDI driver which is
provided by a flashcart vendor has to have 16KB of space reserved,
leaving only 16KB for the remaining code. Thankfully, there's not much
code involved: one needs a lightweight copy of FatFs to read files from
the filesystem, a DLDI patcher for the loaded ARM9 binary, and some
simple load/reset code on top of that.

I've also never created a freestanding NDS homebrew program before, so
I wanted to see what goes into that.

## Porting

Assorted notes:

* `arm9.bin` and `arm7.bin` are extracted from the ELF file as they are
  position-independent - the binaries relocate themselves upon execution
  to areas outside of main RAM, they just have to be started from their
  first word (offset 0).
* Initiailization is deliberately sparse; if a given device needs
  additional cleanup, please document it!

## License

The `miniboot` source code itself is covered under a mix of two licenses:

* the Zlib license,
* the FatFs license.

Individual binaries for specific flashcarts may be covered under their own
respective licenses.
