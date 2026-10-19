#include <cstdio>
#include <cuELF.h>
#include <nvinfo.h>
#include <string.h>

int main(int argc, const char * argv[]) {
    
    if (argc < 2) {
        printf("Usage: %s <elf_file>\n", argv[0]);
        return 1;
    }

    const char *elf_file = argv[1];
    cuELF elf(elf_file);

    size_t len = elf.get_num_sections();
    for (size_t i = 0; i < len; i++) {
        cuELFSection *section = elf.get_section(i);
        if (section->header.sh_type == SHT_LOPROC
        && strncmp(section->name, ".nv.info", 8) == 0) {
            cuELFInfoSection nv_info(section);
            printf("Section %s\n", section->name);
            nv_info.info->dump();
            printf("\n");
        }
    }

    return 0;
}