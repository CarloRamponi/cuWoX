#include <cstdio>
#include <cuELF.h>
#include <nvinfo.h>
#include <string.h>
#include <cuInstruction.h>
#include <log.h>

int main(int argc, const char * argv[]) {

    if (argc < 2) {
        printf("Usage: %s <input_file> [<output_file>]\n", argv[0]);
        return 1;
    }

    const char *output;

    if (argc < 3) {
        printf("Output file not specified, using default: /tmp/output.elf\n");
        output = "/tmp/output.elf";
    } else {
        output = argv[2];
    }

    const char *elf_file = argv[1];
    cuELF *elf = cuELF::from_file(elf_file);

    // elf.dump_header();
    // elf.dump_segments();
    // elf.dump_sections();
    // elf.dump_relocations();
    // elf.dump_symbols();
    elf->instrument_sections(0x7fffdf000000, 0x2000000);

    elf->print_stats();

    elf->write_elf(output);

    delete elf;
    return 0;
}
