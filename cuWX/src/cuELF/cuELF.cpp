#include <cuELF.h>
#include <stdexcept>
#include <string.h>
#include <log.h>
#include <cuInstruction.h>
#include <algorithm>

cuELF * cuELF::from_file(const char *elf_file) {
    LOG("Loading ELF file: %s\n", elf_file);

    FILE *file = fopen(elf_file, "rb");
    if (!file) {
        LOG_ERROR("Failed to open file: %s\n", elf_file);
        throw std::runtime_error("Failed to open file");
    }

    // Allocate memory for the whole ELF file
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    uint8_t *file_data = new uint8_t[file_size];
    if (fread(file_data, 1, file_size, file) != file_size) {
        fclose(file);
        delete[] file_data;
        LOG_ERROR("Failed to read file: %s\n", elf_file);
        throw std::runtime_error("Failed to read file");
    }

    cuELF *elf = new cuELF(file_data);
    
    delete[] file_data;
    fclose(file);
    return elf;
}

cuELF::cuELF(const uint8_t *data) {
    this->init(data);
}

void cuELF::init(const uint8_t *data) {
    this->init_header(data);

    // Check if the file is an ELF file
    if (header.e_ident[0] != ELFMAG0 || header.e_ident[1] != ELFMAG1 || header.e_ident[2] != ELFMAG2 || header.e_ident[3] != ELFMAG3) {
        LOG_ERROR("Not an ELF file\n");
        throw std::runtime_error("Not an ELF file");
    }

    // Check if the file is a 64-bit ELF file
    if (header.e_ident[EI_CLASS] != ELFCLASS64) {
        LOG_ERROR("Not a 64-bit ELF file\n");
        throw std::runtime_error("Not a 64-bit ELF file");
    }

    // Check if the file is a little-endian ELF file
    if (header.e_ident[EI_DATA] != ELFDATA2LSB) {
        LOG_ERROR("Not a little-endian ELF file\n");
        throw std::runtime_error("Not a little-endian ELF file");
    }

    // // Check if the file is an executable ELF file
    // if (header.e_type != ET_EXEC) {
    //     LOG_ERROR("Not an executable ELF file\n");
    //     throw std::runtime_error("Not an executable ELF file");
    // }

    // Check if the file is for CUDA architecture
    if (header.e_machine != EM_CUDA) {
        LOG_ERROR("Not a CUDA ELF file\n");
        throw std::runtime_error("Not a CUDA ELF file");
    }

    this->sm_version = (header.e_flags >> 16) & 0xFF;
    if(this->sm_version == 0) {
        this->sm_version = (header.e_flags >> 8) & 0xFF;
    }
    LOG("SM VERSION: sm_%u\n", this->sm_version);

    this->init_sections(data);
    this->init_segments(data);
    this->init_symbols(data);
}

void cuELF::init_header(const uint8_t *data){
    memcpy(&header, data, sizeof(header));
}

void cuELF::init_sections(const uint8_t *data){
    // Read the section header table
    Elf64_Shdr s_hdr;
    
    for (int i = 0; i < header.e_shnum; i++) {
        memcpy(&s_hdr, data + header.e_shoff + i * header.e_shentsize, sizeof(Elf64_Shdr));
        
        cuELFSection section;
        section.header = s_hdr;
        section.offset = s_hdr.sh_offset;
        section.size = s_hdr.sh_size;

        if(s_hdr.sh_type == SHT_NOBITS) {
            section.data = nullptr;
        } else {
            try {
                section.data = new char[s_hdr.sh_size];
            } catch (const std::bad_alloc &e) {
                LOG_ERROR("Failed to allocate memory for section %d of size 0x%lx: %s\n", i, s_hdr.sh_size, e.what());
                throw;
            }
            memcpy(section.data, data + s_hdr.sh_offset, s_hdr.sh_size);
        }

        this->sections.push_back(section);
    }

    char * shstrtab_data = this->sections[header.e_shstrndx].data;
    
    for (int i = 0; i < header.e_shnum; i++) {
        const char *name = shstrtab_data + this->sections[i].header.sh_name;
        this->sections[i].name = name;
    }
}

void cuELF::init_segments(const uint8_t *data) {
    // Read the program header table
    for (int i = 0; i < header.e_phnum; i++) {
        Elf64_Phdr phdr;
        memcpy(&phdr, data + header.e_phoff + i * header.e_phentsize, sizeof(Elf64_Phdr));

        cuELFSegment segment;
        segment.header = phdr;

        if (phdr.p_offset == header.e_phoff) {
            segment.ph_segement = true;
        } else {
        
            // Get sections sorted by offset to find segments correctly
            auto sorted_indices = get_sections_sorted_by_offset();
            uint64_t phdr_file_end = phdr.p_offset + phdr.p_filesz;
            uint64_t phdr_mem_end = phdr.p_offset + phdr.p_memsz;
            
            // Find first section at phdr.p_offset
            for (int idx : sorted_indices) {
                if (phdr.p_offset == this->sections[idx].offset) {
                    segment.sec_start = idx;
                    break;
                }
            }

            if (segment.sec_start == -1) {
                LOG_ERROR("Failed to find start section for program header %d\n", i);
                throw std::runtime_error("Failed to find section for program header");
            }

            // Find last file-backed section and last in-memory section covered by this program header
            for (int idx : sorted_indices) {
                uint64_t sec_start = this->sections[idx].offset;
                uint64_t sec_end = sec_start + this->sections[idx].size;

                if (sec_start >= phdr.p_offset && sec_start < phdr_file_end && this->sections[idx].header.sh_type != SHT_NOBITS) {
                    segment.sec_end = idx;
                }

                if (sec_start >= phdr.p_offset && sec_start < phdr_mem_end) {
                    segment.sec_mem_end = idx;
                }
            }

            if (segment.sec_end == -1) {
                LOG_WARNING("Failed to find end section for program header %d\n", i);
                // Find the last non-NOBITS section in sorted order that hasn't been assigned
                for (int j = (int)sorted_indices.size() - 1; j >= 0; j--) {
                    if (this->sections[sorted_indices[j]].header.sh_type != SHT_NOBITS) {
                        segment.sec_end = sorted_indices[j];
                        break;
                    }
                }
            }

            if (segment.sec_mem_end == -1) {
                segment.sec_mem_end = segment.sec_end;
            }
        }

        this->segments.push_back(segment);
    }
}

void cuELF::init_symbols(const uint8_t *data) {
    int symtab_idx = get_section_by_name(".symtab");
    if (symtab_idx < 0) {
        LOG_ERROR("No .symtab found\n");
        throw std::runtime_error("No .symtab found");
    }
    this->symtab_idx = symtab_idx;

    int strtab_idx = get_section_by_name(".strtab");
    if (strtab_idx < 0) {
        LOG_ERROR("No .strtab found\n");
        throw std::runtime_error("No .strtab found");
    }
    this->strtab_idx = strtab_idx;

    Elf64_Sym *symtab_data = (Elf64_Sym *)this->sections[symtab_idx].data;
    for (unsigned long int i = 0; i < this->sections[symtab_idx].size / sizeof(Elf64_Sym); i++) {
        cuELFSymbol symbol;
        symbol.sym = symtab_data[i];
        symbol.name = this->sections[strtab_idx].data + symtab_data[i].st_name;
        this->symbols.push_back(symbol);
    }
}

uint32_t cuELF::get_symbol_by_name(const char *name) {
    for (int i = 0; i < this->header.e_shnum; i++) {
        if (strcmp(this->symbols[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

cuELF::~cuELF() {
    for (auto section : this->sections) {
        if (section.data) {
            delete[] section.data;
        }
    }
}

int32_t cuELF::get_section_by_name(const char *name) {
    for (int i = 0; i < this->header.e_shnum; i++) {
        if (strcmp(this->sections[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}


cuELFSection *cuELF::get_section(Elf64_Section index) {
    if (index < 0 || index >= this->header.e_shnum) {
        return nullptr;
    }
    return &this->sections[index];
}

size_t cuELF::get_elf_size() {
    this->update();
    return this->header.e_phoff + this->header.e_phnum * sizeof(Elf64_Phdr);
}

void cuELF::export_bytes(uint8_t *dst) {
    size_t size = this->get_elf_size();
    memset(dst, 0, size);
    
    // Write the ELF header
    memcpy(dst, &this->header, sizeof(this->header));

    for (int i = 0; i < this->header.e_shnum; i++) {
        if (this->sections[i].data && this->sections[i].header.sh_type != SHT_NOBITS) {
            memcpy(dst + this->sections[i].offset, this->sections[i].data, this->sections[i].size);
        }
    }

    for (int i = 0; i < this->header.e_shnum; i++) {
        memcpy(dst + this->header.e_shoff + i * this->header.e_shentsize, &this->sections[i].header, this->header.e_shentsize);
    }

    for (int i = 0; i < this->header.e_phnum; i++) {
        memcpy(dst + this->header.e_phoff + i * this->header.e_phentsize, &this->segments[i].header, this->header.e_phentsize);
    }
}

void cuELF::write_elf(const char *filename)
{
    LOG_VERBOSE("Writing ELF file: %s\n", filename);
    FILE *file = fopen(filename, "wb");
    if (!file) {
        LOG_ERROR("Failed to open file: %s\n", filename);
        throw std::runtime_error("Failed to open file");
    }

    size_t size = this->get_elf_size();

    uint8_t * output_buf = new uint8_t[size];
    this->export_bytes(output_buf);
    fwrite(output_buf, size, 1, file);
    fclose(file);
    delete[] output_buf;
}

void cuELF::update() {
    // order is important
    this->update_symbols();
    this->update_sections();
    this->update_segments();
    this->update_header();
}

void cuELF::update_header() {
    this->header.e_shnum = this->sections.size();
    this->header.e_phnum = this->segments.size();
    this->header.e_shstrndx = this->get_section_by_name(".shstrtab");

    size_t shoff = 0;
    for (const auto &section : this->sections) {
        if (section.header.sh_type == SHT_NOBITS) {
            continue;
        }
        shoff = std::max(shoff, section.offset + section.size);
    }

    this->header.e_shoff = shoff;
    this->header.e_phoff = shoff + (this->sections.size() * sizeof(Elf64_Shdr));
}

std::vector<int32_t> cuELF::get_sections_sorted_by_offset() {
    std::vector<int32_t> indices;
    for (int i = 0; i < (int)this->sections.size(); i++) {
        indices.push_back(i);
    }
    // Sort by offset
    std::sort(indices.begin(), indices.end(), [this](int32_t a, int32_t b) {
        if (this->sections[a].offset != this->sections[b].offset) {
            return this->sections[a].offset < this->sections[b].offset;
        }
        return a < b;
    });
    return indices;
}

void cuELF::update_sections() {
    // First section is always the null section, it has to be at offset 0 and size 0
    this->sections[0].offset = 0;
    this->sections[0].header.sh_offset = 0;
    this->sections[0].header.sh_size = 0;
    
    size_t offset = sizeof(Elf64_Ehdr);
    
    // Get sections sorted by offset to properly layout the file
    auto sorted_indices = get_sections_sorted_by_offset();
    
    for (size_t i = 1; i < sorted_indices.size(); ) {
        int idx = sorted_indices[i];
        size_t group_end = i + 1;

        while (group_end < sorted_indices.size()) {
            int next_idx = sorted_indices[group_end];
            if (this->sections[next_idx].offset != this->sections[idx].offset ||
                this->sections[next_idx].size != this->sections[idx].size ||
                this->sections[next_idx].header.sh_type != this->sections[idx].header.sh_type) {
                break;
            }
            group_end++;
        }

        // LOG_VERBOSE("Updating section %s\nCurrent offset: 0x%lx\n", this->sections[idx].name, offset);
        // handle alignment
        if (this->sections[idx].header.sh_addralign > 1) {
            offset = align(offset, this->sections[idx].header.sh_addralign);
        }

        if (idx == 2) {
            offset += 0x24; // Don't ask me why
        }

        for (size_t j = i; j < group_end; j++) {
            int group_idx = sorted_indices[j];
            this->sections[group_idx].offset = offset;
            this->sections[group_idx].header.sh_offset = offset;
            this->sections[group_idx].header.sh_size = this->sections[group_idx].size;
        }

        // NOBITS sections don't have data in the file, so they don't take space
        if(this->sections[idx].header.sh_type != SHT_NOBITS) {
            offset += this->sections[idx].size;
        }
        // LOG_VERBOSE("Section %s: offset=0x%lx, size=0x%lx, new_offset=0x%lx\n", this->sections[idx].name, this->sections[idx].offset, this->sections[idx].size, offset);

        i = group_end;
    }
}

void cuELF::update_segments() {
    auto sorted_indices = get_sections_sorted_by_offset();

    for (size_t i = 0; i < this->segments.size(); i++) {
        if (this->segments[i].ph_segement) {
            size_t file_end = 0;
            for (int idx : sorted_indices) {
                if (this->sections[idx].header.sh_type != SHT_NOBITS) {
                    file_end = std::max(file_end, this->sections[idx].offset + this->sections[idx].size);
                }
            }
            this->segments[i].header.p_offset = file_end + this->sections.size() * sizeof(Elf64_Shdr);
        } else {
            this->segments[i].header.p_offset = this->sections[this->segments[i].sec_start].offset;
            this->segments[i].header.p_filesz = this->sections[this->segments[i].sec_end].offset + this->sections[this->segments[i].sec_end].size - this->segments[i].header.p_offset;
            this->segments[i].header.p_memsz = this->sections[this->segments[i].sec_mem_end].offset + this->sections[this->segments[i].sec_mem_end].size - this->segments[i].header.p_offset;
        }
    }
}

void cuELF::update_section_names() {
    for (size_t i = 0; i < this->sections.size(); i++) {
        this->sections[i].name = this->sections[this->header.e_shstrndx].data + this->sections[i].header.sh_name;
    }
}

void cuELF::update_symbol_names() {
    for (size_t i = 0; i < this->symbols.size(); i++) {
        this->symbols[i].name = this->sections[this->strtab_idx].data + this->symbols[i].sym.st_name;
    }
}

void cuELF::update_symbols() {
    if (this->symtab_idx < 0) {
        return;
    }

    cuELFSection &symsec = this->sections[this->symtab_idx];
    symsec.header.sh_size = this->symbols.size() * sizeof(Elf64_Sym);
    symsec.size = symsec.header.sh_size;
    delete[] symsec.data;
    symsec.data = new char[symsec.header.sh_size];
    for (size_t i = 0; i < this->symbols.size(); i++) {
        memcpy(symsec.data + i * sizeof(Elf64_Sym), &this->symbols[i].sym, sizeof(Elf64_Sym));
    }
}

Elf64_Word cuELF::add_section_name(const char *name) {
    char *shstrtab_data = this->sections[this->header.e_shstrndx].data;
    Elf64_Word offset = this->sections[this->header.e_shstrndx].size;
    size_t new_size = offset + strlen(name) + 1;
    char *new_shstrtab_data = new char[new_size];
    memcpy(new_shstrtab_data, shstrtab_data, offset);
    memcpy(new_shstrtab_data + offset, name, strlen(name) + 1);
    delete[] this->sections[this->header.e_shstrndx].data;
    this->sections[this->header.e_shstrndx].data = new_shstrtab_data;
    this->sections[this->header.e_shstrndx].size = new_size;
    this->update_section_names();
    return offset;
}

Elf64_Word cuELF::add_symbol_name(const char *name) {
    char *strtab_data = this->sections[this->strtab_idx].data;
    Elf64_Word offset = this->sections[this->strtab_idx].size;
    size_t new_size = offset + strlen(name) + 1;
    char *new_strtab_data = new char[new_size];
    memcpy(new_strtab_data, strtab_data, offset);
    memcpy(new_strtab_data + offset, name, strlen(name) + 1);
    delete[] this->sections[this->strtab_idx].data;
    this->sections[this->strtab_idx].data = new_strtab_data;
    this->sections[this->strtab_idx].size = new_size;
    this->update_symbol_names();
    return offset;
}

uint32_t cuELF::add_symbol(
    const char *name,
    unsigned char bind,
    unsigned char type,
    unsigned char st_other,
    Elf64_Section st_shndx,
    Elf64_Addr st_value,
    Elf64_Xword st_size
) {
    if (this->symtab_idx < 0) {
        throw std::runtime_error("No .symtab found");
    }

    Elf64_Sym sym;
    sym.st_name = this->add_symbol_name(name);
    sym.st_info = ELF64_ST_INFO(bind, type);
    sym.st_other = st_other;
    sym.st_shndx = st_shndx;
    sym.st_value = st_value;
    sym.st_size = st_size;

    cuELFSection &symsec = this->sections[this->symtab_idx];

    cuELFSymbol symbol;
    symbol.sym = sym;
    symbol.name = symsec.data + sym.st_name;

    uint32_t index = this->symbols.size();
    if (bind == STB_LOCAL) {
        index = symsec.header.sh_info;
        symsec.header.sh_info++;
    }

    this->symbols.insert(this->symbols.begin() + index, symbol);

    // this->fix_symbol_indexes(index);

    return index;
}

void cuELF::fix_symbol_indexes(uint32_t index) {
    for (size_t i = 0; i < this->sections.size(); i++) {
        if (this->sections[i].header.sh_type == SHT_REL) {
            Elf64_Rel *rel_data = (Elf64_Rel *)this->sections[i].data;
            for (size_t j = 0; j < this->sections[i].size / sizeof(Elf64_Rel); j++) {
                if (ELF64_R_SYM(rel_data[j].r_info) >= index) {
                    rel_data[j].r_info = ELF64_R_INFO(ELF64_R_SYM(rel_data[j].r_info) + 1, ELF64_R_TYPE(rel_data[j].r_info));
                }
            }
        } else if (this->sections[i].header.sh_type == SHT_RELA) {
            Elf64_Rela *rela_data = (Elf64_Rela *)this->sections[i].data;
            for (size_t j = 0; j < this->sections[i].size / sizeof(Elf64_Rela); j++) {
                if (ELF64_R_SYM(rela_data[j].r_info) >= index) {
                    rela_data[j].r_info = ELF64_R_INFO(ELF64_R_SYM(rela_data[j].r_info) + 1, ELF64_R_TYPE(rela_data[j].r_info));
                }
            }
        }
    }

    for (size_t i = 0; i < this->sections.size(); i++) {
        // .nv.info* sections
        cuELFSection *section = &this->sections[i];
        if (section->header.sh_type == SHT_LOPROC) {
            cuELFInfoSection nv_info(section);
            for (auto &attr : nv_info.info->attributes) {
                switch (attr.attr) {
                    case EIATTR_REGCOUNT:
                        attr.s_value[0]++;
                        break;
                    case EIATTR_FRAME_SIZE:
                        attr.s_value[0]++;
                        break;
                    case EIATTR_MIN_STACK_SIZE:
                        attr.s_value[0]++;
                        break;
                    case EIATTR_MAX_STACK_SIZE:
                        attr.s_value[0]++;
                        break;
                    case EIATTR_MAX_THREADS:
                        attr.s_value[0]++;
                        break;
                    case EIATTR_EXTERNS:
                        for(size_t i = 0; i < attr.s_count; i++) {
                            attr.s_value[i]++;
                        }
                        break;
                    default:
                        break;
                    // there are probably more
                }
            }
            nv_info.update();
        }
    }

    // .text info header
    for (auto &section : this->sections) {
        if (section.header.sh_type == SHT_PROGBITS && strncmp(section.name, ".text.", 6) == 0) {
            uint8_t sym_idx = section.header.sh_info & 0xff;
            if (sym_idx >= index) {
                section.header.sh_info++;
            }
        }
    }

    // TODO maybe fix indexes somewhere else
}

std::vector<cuELFSection> &cuELF::get_sections() {
    return this->sections;
}

void cuELF::instrument_sections(uint64_t code_page, size_t code_page_size) {

    // std::vector<std::pair<uint16_t, size_t>> executable_sections;
    // for (size_t i = 0; i < this->sections.size(); i++) {
    //     if (this->sections[i].header.sh_type == SHT_PROGBITS
    //         && strncmp(this->sections[i].name, ".text.", 6) == 0) {
    //         int16_t symbol_idx = -1;
    //         for(uint j = 0; j < this->symbols.size(); j++) {
    //             auto &symbol = this->symbols[j];
    //             if (symbol.sym.st_shndx == i
    //                 && ELF64_ST_TYPE(symbol.sym.st_info) == STT_FUNC) {
    //                 symbol_idx = j;
    //                 break;
    //             }
    //         }
    //         if (symbol_idx < 0) {
    //             LOG("Failed to find FUNC symbol for section %s", this->sections[i].name);
    //         } else {
    //             executable_sections.push_back({ (uint16_t)symbol_idx, this->sections[i].size });
    //         }
    //     }
    // }

    for (size_t i = 0; i < this->sections.size(); i++) {
        cuELFSection &section = this->sections[i];
        if (section.header.sh_type == SHT_PROGBITS
            && strncmp(section.name, ".text.", 6) == 0) {
            this->instrument_section(i, code_page, code_page_size);
        }
    }
}

void cuELF::instrument_section(Elf64_Section index, uint64_t code_page, size_t code_page_size) {
    cuELFSection &section = this->sections[index];
    char *data = section.data;
    size_t size = section.size;

    this->code_size_original += size;

    size_t instr_count = size / sizeof(cuInstruction::Instruction);
    cuInstruction::Instruction *instructions = (cuInstruction::Instruction *)data;

    // for (size_t i = 0; i < instr_count; i++) {
    //     instructions[i].print(i * sizeof(cuInstruction::Instruction));
    // }

    std::set<uint8_t> loaded_registers = get_loaded_registers(instructions, instr_count);

    // Keep a list of return instructions
    std::vector<std::pair<uint64_t, cuInstruction::Instruction *>> return_instructions;
    for (size_t i = 0; i < instr_count; i++) {
        if (instructions[i].opcode == cuInstruction::OPCODE::RET) {
            size_t offset = i * sizeof(cuInstruction::Instruction);
            this->ret_count++;
            if (loaded_registers.contains(instructions[i].ret.reg_src)) {
                LOG_VERBOSE("Return instruction at offset: 0x%04lx\n", offset);
                this->instrumented_ret_count++;
                #ifdef VERBOSE
                        instructions[i].print(offset);
                #endif
                return_instructions.push_back({ offset, &instructions[i] });
            } else {
                LOG_VERBOSE("Skipping return instruction at offset 0x%04lx, register %d not loaded\n", offset, instructions[i].ret.reg_src);
            }
        }
    }

    size_t cursor = section.size;
    std::vector<cuInstruction::Instruction> instrumentation;

    // Move the cursor back to overwrite the NOP padding
    while (cursor > 0 && instructions[(cursor / sizeof(cuInstruction::Instruction)) - 1].opcode == cuInstruction::OPCODE::NOP) {
        cursor -= sizeof(cuInstruction::Instruction);
    }
    LOG_VERBOSE("Cursor position: 0x%04lx\n", cursor);

    this->instr_count_original += cursor / sizeof(cuInstruction::Instruction);
    this->instr_count_instrumented += cursor / sizeof(cuInstruction::Instruction);

    if (return_instructions.empty()) {
        LOG_VERBOSE("No return instructions found in section %s\n", section.name);
        this->code_size_instrumented += size;
        return;
    }

    size_t instrumentation_start = cursor;

    // Find the rela section for the current section
    // int rela_index = -1;
    // for (size_t i = 0; i < this->sections.size(); i++) {
    //     if (this->sections[i].header.sh_type == SHT_RELA
    //         && this->sections[i].header.sh_info == index) {
    //         rela_index = i;
    //         break;
    //     }
    // }
    // if (rela_index < 0) {
    //     LOG("Failed to find RELA section for section %s\n", section.name);
    //     return;
    // }

    for(const auto &iter : return_instructions) {
        cuInstruction::Instruction *ret_instr = iter.second;

        // Create a branch instruction to the cursor
        cuInstruction::Instruction bra_instr = cuInstruction::BRA::create(ret_instr->pred);
        if(this->sm_version < 90) {
            bra_instr.bra.set_offset80(iter.first, cursor);
        } else {
            bra_instr.bra.set_offset90(iter.first, cursor);
        }
        
        if (ret_instr->ret.flag_ABS) {
            // Absolute return

            // P0 = ret_lo >= lim_inf
            instrumentation.push_back(cuInstruction::ISETP::create(
                cuInstruction::PRED::PT,
                ret_instr->ret.reg_src,
                code_page & 0xFFFFFFFF,
                0, 0, 0, 0, 0, 1, 1,
                cuInstruction::PRED::P6
            ));
            // this->add_rela(rela_index, cursor, ex_sec.first, R_CUDA_ABS32_LO_32, 0);
            cursor += sizeof(cuInstruction::Instruction);

            // P0 = P0 && ret_lo < lim_inf + size
            instrumentation.push_back(cuInstruction::ISETP::create(
                cuInstruction::PRED::PT,
                ret_instr->ret.reg_src,
                (code_page + code_page_size) & 0xFFFFFFFF,
                0, 0, 0, 0, 1, 0, 0,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::P6
            ));
            // this->add_rela(rela_index, cursor, ex_sec.first, R_CUDA_ABS32_LO_32, ex_sec.second);
            cursor += sizeof(cuInstruction::Instruction);

            // P0 = P0 && ret_hi == base_high
            instrumentation.push_back(cuInstruction::ISETP::create(
                cuInstruction::PRED::PT,
                ret_instr->ret.reg_src + 1,
                code_page >> 32,
                0, 0, 0, 0, 0, 1, 0,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::P6
            ));
            // this->add_rela(rela_index, cursor, ex_sec.first, R_CUDA_ABS32_HI_32, 0);
            cursor += sizeof(cuInstruction::Instruction);

            instrumentation.push_back(cuInstruction::RET::create(
                cuInstruction::PRED::P6,
                ret_instr->ret.reg_src,
                1, 1
            ));
            cursor += sizeof(cuInstruction::Instruction);

            instrumentation.push_back(cuInstruction::TRAP::create(
                cuInstruction::PRED::PT,
                0x1
            ));
            cursor += sizeof(cuInstruction::Instruction);

            auto self_loop = cuInstruction::BRA::create();
            if(this->sm_version < 90) {
                self_loop.bra.set_offset80(cursor, cursor);
            } else {
                self_loop.bra.set_offset90(cursor, cursor);
            }
            instrumentation.push_back(self_loop);
            cursor += sizeof(cuInstruction::Instruction);

        } else {
            // Relative return

            // Check that the contents of the register based on the offset and the original instruction's position,
            uint64_t offset;
            if(this->sm_version < 90) {
                offset = ret_instr->ret.get_offset80(iter.first);
            } else {
                offset = ret_instr->ret.get_offset90(iter.first);
            }

            LOG_VERBOSE("Relative return offset 0x%lx\n", offset);
            uint64_t lim_inf = 0 - offset;
            uint64_t lim_sup = size - offset;
            LOG_VERBOSE("Range: [0x%lx, 0x%lx]\n", lim_inf, lim_sup);

            instrumentation.push_back(cuInstruction::ISETP::create(
                cuInstruction::PRED::PT,
                ret_instr->ret.reg_src,
                lim_inf,
                0, 0, 0, 0, 0, 1, 1,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::PT,
                cuInstruction::PRED::PT
            ));
            cursor += sizeof(cuInstruction::Instruction);

            instrumentation.push_back(cuInstruction::ISETP::create(
                cuInstruction::PRED::PT,
                ret_instr->ret.reg_src,
                lim_sup,
                0, 0, 0, 0, 1, 0, 0,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::P6,
                cuInstruction::PRED::PT
            ));
            cursor += sizeof(cuInstruction::Instruction);

            auto new_ret_instr = cuInstruction::RET::create(
                cuInstruction::PRED::P6,
                ret_instr->ret.reg_src,
                0, 1
            );
            if(this->sm_version < 90) {
                new_ret_instr.ret.set_offset80(cursor, offset);
            } else {
                new_ret_instr.ret.set_offset90(cursor, offset);
            }
            instrumentation.push_back(new_ret_instr);
            cursor += sizeof(cuInstruction::Instruction);

            instrumentation.push_back(cuInstruction::TRAP::create(
                cuInstruction::PRED::PT,
                0x1
            ));
            cursor += sizeof(cuInstruction::Instruction);

            auto self_loop = cuInstruction::BRA::create();
            if(this->sm_version < 90) {
                self_loop.bra.set_offset80(cursor, cursor);
            } else {
                self_loop.bra.set_offset90(cursor, cursor);
            }
            instrumentation.push_back(self_loop);
            cursor += sizeof(cuInstruction::Instruction);
        }

        // Replace the return instruction with a branch to the cursor
        *(ret_instr) = bra_instr;
    }

    this->instr_count_instrumented += instrumentation.size();

    // Add the instrumentation to the last section
    // NOP padding
    if (cursor % 0x100 != 0) {
        size_t padding_count = (0x100 - (cursor % 0x100)) / sizeof(cuInstruction::Instruction);
        for (size_t i = 0; i < padding_count; i++) {
            instrumentation.push_back(cuInstruction::NOP::create(cuInstruction::PRED::PT));
        }
    }

    // Add the instrumentation to the section
    char *old_data = section.data;
    size_t new_size = instrumentation_start + instrumentation.size() * sizeof(cuInstruction::Instruction);
    this->code_size_instrumented += std::max(new_size, size);
    if (new_size > size) {
        section.size = new_size;
        section.data = new char[new_size];
        memcpy(section.data, old_data, instrumentation_start);
        delete[] old_data;
        memcpy(section.data + instrumentation_start, instrumentation.data(), instrumentation.size() * sizeof(cuInstruction::Instruction));
    } else {
        memcpy(section.data + instrumentation_start, instrumentation.data(), instrumentation.size() * sizeof(cuInstruction::Instruction));
    }
}

std::set<uint8_t> cuELF::get_loaded_registers(cuInstruction::Instruction *instr, size_t n) {
    std::set<uint8_t> loaded_registers;

    for (size_t i = 0; i < n; i++) {
        if (instr[i].opcode == cuInstruction::OPCODE::LD) {
            loaded_registers.insert(instr[i].ld.reg_dst);
        } else if (instr[i].opcode == cuInstruction::OPCODE::LDL) {
            loaded_registers.insert(instr[i].ldl.reg_dst);
        } else if (instr[i].opcode == cuInstruction::OPCODE::LDG) {
            loaded_registers.insert(instr[i].ldg.reg_dst);
        } else if (instr[i].opcode == cuInstruction::OPCODE::LDC) {
            loaded_registers.insert(instr[i].ldc.reg_dst);
        }
    }
    return loaded_registers;
}

void cuELF::print_stats() {
    printf("Original Code Size: %zu\n", this->code_size_original);
    printf("Instrumented Code Size: %zu\n", this->code_size_instrumented);
    printf("Original Instruction Count (excluding NOP padding): %zu\n", this->instr_count_original);
    printf("Instrumented Instruction Count  (excluding NOP padding): %zu\n", this->instr_count_instrumented);
    printf("Return Instruction Count: %zu\n", this->ret_count);
    printf("Instrumented Return Instruction Count: %zu\n", this->instrumented_ret_count);
}

Elf64_Section cuELF::add_section(
    const char *name,
    Elf64_Word sh_type,
    const char *data,
    Elf64_Xword sh_size,
    int32_t index,
    Elf64_Word sh_flags,
    Elf64_Addr sh_addr,
    Elf64_Word sh_link,
    Elf64_Word sh_info,
    Elf64_Xword sh_addralign,
    Elf64_Xword sh_entsize,
    bool merge_before,
    bool merge_after)
{
    if(index != -1 && (index <= 3 || (size_t)index >= this->sections.size())) {
        throw std::runtime_error("Invalid section index provided");
    }

    Elf64_Shdr sh;
    sh.sh_name = this->add_section_name(name);
    sh.sh_type = sh_type;
    sh.sh_flags = sh_flags;
    sh.sh_addr = sh_addr;
    sh.sh_link = sh_link;
    sh.sh_info = sh_info;
    sh.sh_addralign = sh_addralign;
    sh.sh_entsize = sh_entsize;

    if(index == -1) {
        index = this->header.e_shnum;
    }

    cuELFSection section;
    section.header = sh;
    section.size = sh_size;
    section.data = new char[sh_size];
    section.name = this->sections[this->header.e_shstrndx].data + sh.sh_name;
    memcpy(section.data, data, sh_size);

    this->sections.insert(this->sections.begin() + index, section);
    this->fix_section_indexes(index, merge_before, merge_after);

    return index;
}

void cuELF::fix_section_indexes(Elf64_Section index, bool merge_before, bool merge_after) {

    // update segment boundaries
    for (size_t i = 0; i < this->segments.size(); i++) {
        if (this->segments[i].sec_start > index) {
            this->segments[i].sec_start++;
            this->segments[i].sec_end++;
            this->segments[i].sec_mem_end++;
        } else if (this->segments[i].sec_start == index) {
            if (merge_before) {
                this->segments[i].sec_end++;
                this->segments[i].sec_mem_end++;
            } else {
                this->segments[i].sec_start++;
                this->segments[i].sec_end++;
                this->segments[i].sec_mem_end++;
            }
        } else if(this->segments[i].sec_end == index - 1) {
            if (merge_after) {
                this->segments[i].sec_end++;
            }
        } else if(index > this->segments[i].sec_start && index - 1 < this->segments[i].sec_end) {
            this->segments[i].sec_end++;
        }

        if (this->segments[i].sec_mem_end > index) {
            this->segments[i].sec_mem_end++;
        } else if (this->segments[i].sec_mem_end == index - 1) {
            if (merge_after) {
                this->segments[i].sec_mem_end++;
            }
        } else if (index > this->segments[i].sec_start && index - 1 < this->segments[i].sec_mem_end) {
            this->segments[i].sec_mem_end++;
        }
    }

    // update relocation info
    for (size_t i = 0; i < this->sections.size(); i++) {
        if (i == index) continue;
        if (this->sections[i].header.sh_type == SHT_REL
            || this->sections[i].header.sh_type == SHT_RELA
            || strncmp(this->sections[i].name, ".nv.info.", 9) == 0) {
            if (this->sections[i].header.sh_info >= index) {
                this->sections[i].header.sh_info++;
            }
        }
    }

    for (size_t i = 0; i < this->sections.size(); i++) {
        if (i == index) continue;
        if (this->sections[i].header.sh_link >= index) {
            this->sections[i].header.sh_link++;
        }
    }
    
    // update symbol table info
    for(size_t i = 0; i < this->symbols.size(); i++) {
        if(this->symbols[i].sym.st_shndx >= index) {
            this->symbols[i].sym.st_shndx++;
        }
    }
}

void cuELF::dump_sections() {
    for (int i = 0; i < this->header.e_shnum; i++) {
        this->dump_section(i);
    }
}

void cuELF::dump_segment(uint16_t index) {
    if (index < 0 || index >= this->header.e_phnum) {
        return;
    }

    Elf64_Phdr &ph = this->segments[index].header;
    printf("Segment %d\n", index);
    printf("\tp_type: %d\n", ph.p_type);
    printf("\tp_flags: %u\n", ph.p_flags);
    printf("\tp_offset: %ld\n", ph.p_offset);
    printf("\tp_vaddr: %ld\n", ph.p_vaddr);
    printf("\tp_paddr: %ld\n", ph.p_paddr);
    printf("\tp_filesz: %ld\n", ph.p_filesz);
    printf("\tp_memsz: %ld\n", ph.p_memsz);
    printf("\tp_align: %ld\n", ph.p_align);
    printf("\tSections:\n");
    if (this->segments[index].sec_start > 0) {
        for (int i = this->segments[index].sec_start; i <= this->segments[index].sec_end; i++) {
            printf("\t\t%s\n", this->sections[i].name);
        }
    } else {
        printf("\t\tNone\n");
    }
}

void cuELF::dump_segments() {
    for (int i = 0; i < this->header.e_phnum; i++) {
        this->dump_segment(i);
    }
}

void cuELF::dump_header() {
    printf("ELF Header\n");
    printf("\te_type: %d\n", this->header.e_type);
    printf("\te_machine: %d\n", this->header.e_machine);
    printf("\te_version: %d\n", this->header.e_version);
    printf("\te_entry: %ld\n", this->header.e_entry);
    printf("\te_phoff: %ld\n", this->header.e_phoff);
    printf("\te_shoff: %ld\n", this->header.e_shoff);
    printf("\te_flags: %u\n", this->header.e_flags);
    printf("\te_ehsize: %d\n", this->header.e_ehsize);
    printf("\te_phentsize: %d\n", this->header.e_phentsize);
    printf("\te_phnum: %d\n", this->header.e_phnum);
    printf("\te_shentsize: %d\n", this->header.e_shentsize);
    printf("\te_shnum: %d\n", this->header.e_shnum);
    printf("\te_shstrndx: %d\n", this->header.e_shstrndx);
}

void cuELF::add_rel(
    Elf64_Section rel_section,
    Elf64_Addr offset,
    Elf64_Word sym,
    Elf64_Word type
) {
    Elf64_Rel rel;
    rel.r_offset = offset;
    rel.r_info = ELF64_R_INFO(sym, type);

    cuELFSection &section = this->sections[rel_section];
    if (section.header.sh_type != SHT_REL) {
        throw std::runtime_error("Section is not a relocation section");
        return;
    }

    size_t size = section.size + sizeof(Elf64_Rel);
    char *new_data = new char[size];
    memcpy(new_data, section.data, section.size);
    memcpy(new_data + section.size, &rel, sizeof(Elf64_Rel));
    delete[] section.data;
    section.data = new_data;
    section.size = size;
}

void cuELF::add_rela(
    Elf64_Section rel_section,
    Elf64_Addr offset,
    Elf64_Word sym,
    Elf64_Word type,
    Elf64_Sxword r_addend
) {
    Elf64_Rela rel;
    rel.r_offset = offset;
    rel.r_info = ELF64_R_INFO(sym, type);
    rel.r_addend = r_addend;

    cuELFSection &section = this->sections[rel_section];
    if (section.header.sh_type != SHT_RELA) {
        throw std::runtime_error("Section is not a relocation section");
        return;
    }

    size_t size = section.size + sizeof(Elf64_Rela);
    char *new_data = new char[size];
    memcpy(new_data, section.data, section.size);
    memcpy(new_data + section.size, &rel, sizeof(Elf64_Rela));
    delete[] section.data;
    section.data = new_data;
    section.size = size;
}

void cuELF::dump_section(Elf64_Section index) {
    if (index < 0 || index >= this->header.e_shnum) {
        return;
    }

    Elf64_Shdr &sh = this->sections[index].header;
    // dump section header
    if(this->sections[index].name[0] == '\0') {
        printf("Section %d: NULL\n", index);
    } else {
        printf("Section %d: %s\n", index, this->sections[index].name);
    }
    printf("\tsh_name: %d\n", sh.sh_name);
    printf("\tsh_type: %d\n", sh.sh_type);
    printf("\tsh_flags: %ld\n", sh.sh_flags);
    printf("\tsh_addr: %ld\n", sh.sh_addr);
    printf("\tsh_offset: %ld\n", sh.sh_offset);
    printf("\tsh_size: %ld\n", sh.sh_size);
    printf("\tsh_link: %d\n", sh.sh_link);
    printf("\tsh_info: %d\n", sh.sh_info);
    printf("\tsh_addralign: %ld\n", sh.sh_addralign);
    printf("\tsh_entsize: %ld\n", sh.sh_entsize);
}

void cuELF::dump_rel(Elf64_Section index) {
    Elf64_Shdr &sh = this->sections[index].header;
    if (sh.sh_type != SHT_REL) {
        throw std::runtime_error("Section is not a relocation section");
    }

    printf("Relocation section %d (%s)\n", index, this->sections[index].name);
    Elf64_Rel *rel = (Elf64_Rel *)this->sections[index].data;
    for (long unsigned int i = 0; i < sh.sh_size / sizeof(Elf64_Rel); i++) {
        printf("\toffset: 0x%lx\ttype: 0x%lx\tsymbol: %s\n", rel[i].r_offset, ELF64_R_TYPE(rel[i].r_info), this->symbols[ELF64_R_SYM(rel[i].r_info)].name);
    }
}

void cuELF::dump_rela(Elf64_Section index) {
    Elf64_Shdr &sh = this->sections[index].header;
    if (sh.sh_type != SHT_RELA) {
        throw std::runtime_error("Section is not a relocation section");
    }

    printf("Relocation section %d (%s)\n", index, this->sections[index].name);
    Elf64_Rela *rel = (Elf64_Rela *)this->sections[index].data;
    for (long unsigned int i = 0; i < sh.sh_size / sizeof(Elf64_Rela); i++) {
        printf("\toffset: 0x%lx\ttype: 0x%lx\tsymbol (+ offset): %s + 0x%lx\n", rel[i].r_offset, ELF64_R_TYPE(rel[i].r_info), this->symbols[ELF64_R_SYM(rel[i].r_info)].name, rel[i].r_addend);
    }
}

void cuELF::dump_relocations() {
    for (int i = 0; i < this->header.e_shnum; i++) {
        Elf64_Shdr &sh = this->sections[i].header;
        if (sh.sh_type == SHT_REL) {
            this->dump_rel(i);
        } else if (sh.sh_type == SHT_RELA) {
            this->dump_rela(i);
        }
    }
}

void cuELF::dump_symbols() {
    if (this->symtab_idx < 0) {
        throw std::runtime_error("No .symtab found");
    }

    printf("Symbol table\n");
    for (long unsigned int i = 0; i < this->symbols.size(); i++) {
        printf("Symbol %lu: %s\n", i, this->symbols[i].name);
        printf("\tst_name: %d\n", this->symbols[i].sym.st_name);
        printf("\tst_info: %d\n", this->symbols[i].sym.st_info);
        printf("\tst_other: %d\n", this->symbols[i].sym.st_other);
        printf("\tst_shndx: %d\n", this->symbols[i].sym.st_shndx);
        printf("\tst_value: %ld\n", this->symbols[i].sym.st_value);
        printf("\tst_size: %ld\n", this->symbols[i].sym.st_size);
    }
}

size_t align(size_t offset, size_t align) {
    size_t remainder = offset % align;
    if (remainder != 0) {
        offset += align - remainder;
    }
    return offset;
}

cuELFInfoSection::cuELFInfoSection(cuELFSection *section): section(section) {
    if (section->header.sh_type != SHT_LOPROC) {
        throw std::runtime_error("Section is not a PROGBITS section");
    }
    if (strncmp(section->name, ".nv.info", 8) != 0) {
        throw std::runtime_error("Section is not an NV_INFO section");
    }
    this->info = new NVInfo(section->data, section->size);
}

cuELFInfoSection::~cuELFInfoSection() {
    delete this->info;
}

void cuELFInfoSection::add_attribute(const NVInfoAttribute &attr) {
    this->info->add_attribute(attr);
    this->update();
}

void cuELFInfoSection::update() {
    size_t size = this->info->size;
    delete[] this->section->data;
    this->section->data = new char[size];
    this->section->size = size;
    this->info->encode(this->section->data);
}
