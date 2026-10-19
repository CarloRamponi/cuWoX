#ifndef FATBIN_H
#define FATBIN_H

#include <stdint.h>
#include <cuda.h>
#include <elf.h>
#include <fatbinary_section.h>
#include <cuELF.h>
#include <allocations.h>
#include <fatbin_decompress.h>

typedef struct __attribute__((__packed__)) fat_elf_header {
	uint32_t magic;
	uint16_t version;
	uint16_t header_size;
	uint64_t size;
} fat_elf_header_t;


// TODO: fill this structure by RE, and use it to parse the fat binary
typedef struct __attribute__((__packed__)) fat_elf_item {
    uint16_t type;
    uint8_t param_2;
    uint8_t param_3;
    uint32_t header_size;
    uint64_t size;
    uint32_t compressed_size;
    uint32_t param_4;
    uint16_t minor;
    uint16_t major;
    uint32_t compute_version;
    uint32_t obj_name_offset;
    uint32_t obj_name_len;
    uint64_t flags;
    uint64_t zero;                  // Alignment for compression?
    uint64_t uncompressed_size;     // Length of compressed data in decompressed representation.
                                    // There is an uncompressed footer so this is generally smaller
                                    // than size.
    char src_name[];
    // In PTX, followed by (after the end of src_name, aligned to 8 bytes, see get_fatbin_compile_options()):
    // uint32_t compile_options_offset;
    // uint32_t compile_options_len;
    // char compile_options[];
    // Followed by the actual data (PTX or CUBIN)
} fat_elf_item_t;

#define FAT_BIN_ITEM_TYPE_PTX 0x01
#define FAT_BIN_ITEM_TYPE_CUBIN 0x02

#define FATBIN_FLAG_64BIT     0x0000000000000001LL
#define FATBIN_FLAG_DEBUG     0x0000000000000002LL
#define FATBIN_FLAG_LINUX     0x0000000000000010LL
#define FATBIN_FLAG_COMPRESS  0x0000000000002000LL

static int flag_to_str(char** str, uint64_t flag);
static void dump_fatbin_header(fat_elf_item_t *header);

int decompress_fatbin(fat_elf_item_t **item);
int compile_ptx_to_cubin(fat_elf_item_t **item, uint32_t computeCapabilityMajor, uint32_t computeCapabilityMinor);

char *get_fatbin_compile_options(fat_elf_item_t *item, size_t *options_size);

class FatBinaryParser {
public:
    FatBinaryParser(__fatBinC_Wrapper_t *handle);
    void instrument(const Allocation *code_allocation);

private:
    void parse();
    void dump(const char *filename);

    __fatBinC_Wrapper_t *handle;
    size_t total_size = 0;
    std::vector<std::pair<fat_elf_header_t *, std::vector<fat_elf_item_t *>>> elf_files;
    CUmodule _module;
};

#endif // FATBIN_H