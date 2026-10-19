#pragma once

#include <elf.h>
#include <cstdio>
#include <vector>
#include <nvinfo.h>
#include <cuInstruction.h>
#include <set>

enum CUDA_REL_TYPE: uint8_t {
    R_CUDA_32               = 1,
    R_CUDA_64               = 2,
    R_CUDA_G64              = 4,
    R_CUDA_TEX_HEADER_INDEX = 6,
    R_CUDA_SURF_HEADER_INDEX= 52,
    R_CUDA_ABS32_20         = 42,
    R_CUDA_ABS32_LO_20      = 43,
    R_CUDA_ABS32_HI_20      = 44,
    R_CUDA_ABS32_LO_32      = 56,
    R_CUDA_ABS32_HI_32      = 57,
    R_CUDA_ABS47_34         = 58
};

struct cuELFSegment {
    Elf64_Phdr header;
    bool ph_segement = false;
    int32_t sec_start = -1;
    int32_t sec_end = -1;
    int32_t sec_mem_end = -1;
};

struct cuELFSection {
    Elf64_Shdr header;
    char *data;
    const char *name;
    size_t size;
    size_t offset;
};

struct cuELFSymbol {
    Elf64_Sym sym;
    const char *name;
};

class cuELFInfoSection {
    public:
        cuELFInfoSection(cuELFSection *section);
        ~cuELFInfoSection();
        void add_attribute(const NVInfoAttribute &attr);
        NVInfo *info;
        void update();
    
    protected:
        cuELFSection *section;
};

class cuELF {
public:
    cuELF(const uint8_t *data);
    ~cuELF();

    static cuELF *from_file(const char *elf_file);
    
    unsigned int get_num_sections() const { return header.e_shnum; }
    unsigned int get_num_program_headers() const { return header.e_phnum; }
    unsigned int get_num_symbols() const { return header.e_shnum; }
    int32_t get_section_by_name(const char *name);
    cuELFSection *get_section(Elf64_Section index);
    std::vector<cuELFSection> &get_sections();
    uint32_t get_symbol_by_name(const char *name);
    void instrument_sections(uint64_t code_page, size_t code_page_size);

    size_t get_elf_size();
    void export_bytes(uint8_t *dst);
    void write_elf(const char *filename);

    // Stats
    void print_stats();

    Elf64_Section add_section(
        const char *name,
        Elf64_Word sh_type, 
        const char *data,
        Elf64_Xword sh_size,
        int32_t index = -1,
        Elf64_Word sh_flags = 0,
        Elf64_Addr sh_addr = 0,
        Elf64_Word sh_link = 0,
        Elf64_Word sh_info = 0,
        Elf64_Xword sh_addralign = 0,
        Elf64_Xword sh_entsize = 0,
        bool merge_before = false,
        bool merge_after = false
    );

    uint32_t add_symbol(
        const char *name,
        unsigned char bind,
        unsigned char type,
        unsigned char st_other = 0,
        Elf64_Section st_shndx = 0,
        Elf64_Addr st_value = 0,
        Elf64_Xword st_size = 0
    );

    void add_rel(
        Elf64_Section rel_section,
        Elf64_Addr offset,
        Elf64_Word sym,
        Elf64_Word type
    );

    void add_rela(
        Elf64_Section rel_section,
        Elf64_Addr offset,
        Elf64_Word sym,
        Elf64_Word type,
        Elf64_Sxword r_addend
    );

    // void replace_section(Elf64_Section index, const char *data, size_t size);
    
    void dump_section(Elf64_Section index);
    void dump_sections();
    void dump_segment(uint16_t index);
    void dump_segments();
    void dump_header();
    void dump_rel(Elf64_Section index);
    void dump_rela(Elf64_Section index);
    void dump_relocations();
    void dump_symbols();

protected:
    Elf64_Ehdr header;
    std::vector<cuELFSegment> segments;
    std::vector<cuELFSection> sections;
    std::vector<cuELFSymbol> symbols;
    Elf64_Sym *symtab;
    char *strtab;
    char *sh_str_table;
    Elf64_Section symtab_idx = -1;
    Elf64_Section strtab_idx = -1;
    uint32_t sm_version = 0;

    // read elf data from memory
    void init(const uint8_t *data);
    void init_header(const uint8_t *data);
    void init_sections(const uint8_t *data);
    void init_segments(const uint8_t *data);
    void init_symbols(const uint8_t *data);

    // update offsets and sizes before writing
    void update();
    void update_header();
    void update_sections();
    void update_segments();
    void update_symbols();

    // fix string pointers
    void update_section_names();
    void update_symbol_names();

    void fix_section_indexes(Elf64_Section index, bool merge_before = false, bool merge_after = false);
    void fix_symbol_indexes(uint32_t index);

    // Get section indices sorted by their offset in the file
    std::vector<int32_t> get_sections_sorted_by_offset();

    Elf64_Word add_section_name(const char *name);
    Elf64_Word add_symbol_name(const char *name);

    void instrument_section(Elf64_Section index, uint64_t code_page, size_t code_page_size);
    std::set<uint8_t> get_loaded_registers(cuInstruction::Instruction *instr, size_t n);

    // Stats
    size_t code_size_original = 0;
    size_t code_size_instrumented = 0;
    size_t instr_count_original = 0;
    size_t instr_count_instrumented = 0;
    size_t ret_count = 0;
    size_t instrumented_ret_count = 0;
};

size_t align(size_t offset, size_t align);