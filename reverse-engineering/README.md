The code for this port was reverse-engineered from the Windows version of "The Even More! Incredible Machine" (TEMIM).

There were a few reasons to choose the Windows version:

1. The DOS executable is packed, but the Windows executable isn't.
2. The Windows version calls into system DLLS (GDI, USER, KERNEL). This means that code that relates to graphics (GDI) are easily identifable, and we can work backwards (a bottom-up approach).
3. I don't suspect the Windows version is _too_ different from the DOS version. Differences in stuff like the resource file formats can be reconciled.


Hashes, checksums for TEMIM.EXE:

* MD5: 30c97cd68e4ef7c7a35b68c1654d64fe
* SHA1: a42242f504d3101f973711636ff0bcbf5851a772
* SHA256: 03d56a132ff7c987488c6d28cc6ba9c4a28b6f9d085c53a3c5a0bfdd14e49e35


## Runtime analysis: DOSBoxX

Basically: Install Windows 3.1 in DosBoxX, install the game, run it, use the debugger tools.

The DosBoxX debugger shows code and data using protected-mode addresses.

Due to the way that Windows 3.1 loads executables into memory, the selector of each program segment changes each time the executable is loaded. That's why it's important to identify the program's selectors.


## Static analysis: Ghidra

The scripts here are used with Ghidra (version 9.1.2).

Ghidra supports 16-bit Windows executables (NE format) passibly well. Other tools like radare2 fail to patch relocation addresses (when I tried it).

Ghidra still needs a lot of hand-holding to make static analysis of TEMIM.EXE workable. One issue is that Ghidra is unaware of imported function names from GDI, USER and KERNEL. It refers to them by ordinal numbers, e.g. Ordinal1, Ordinal 123, etc.

I'm unable to publish the project files on GitHub, because it would include the entire disassembly, which is similar to sharing the proprietary executable. So instead, I'll publish my findings and relevant code addresses.

