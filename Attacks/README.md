# Attacks

This directory contains the implementations of the proof-of-concept (PoC) demonstrating the two attacks:

- [Shellcode Injection into Data Memory](./code_injection_poc/README.md) — [Demo Video](https://youtu.be/zbYwFJd_UFc)
- [Direct Overwrite of Code Memory](./code_pages_poc/README.md) — [Demo Video](https://youtu.be/NQVI52LDoXg)

## Build Instructions

You can build either PoC using the provided `Makefile`. Run the following command with the desired target folder:

```bash
make DIR=<target_folder_path>
```