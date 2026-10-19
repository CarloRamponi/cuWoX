This tool disassembles a raw binary file containing SM80 instructions.

`nvdisasm` can do the same thing by using the `--binary SM80` flag, but it will stop (and not print anything) if an invalid instruction is found anywhere in the binary file. This is problematic if you have, for instance, a memory dump containing both instructions and data.

This tool takes each 16-bytes block and disassembles it independently, skipping any non valid instruction.