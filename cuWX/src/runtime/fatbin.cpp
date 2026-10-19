#include <fatbin.h>
#include <log.h>
#include <cstring>
#include <cuda_runtime_api.h>
#include <tuple>
#include <hexdump.h>
#include <nvPTXCompiler.h>

FatBinaryParser::FatBinaryParser(__fatBinC_Wrapper_t *handle): handle(handle) {
    parse();
}

void FatBinaryParser::parse() {
    if (!handle) {
        fprintf(stderr, "Error: Invalid fat binary handle.\n");
        return;
    }

    uint8_t *tail = (uint8_t *)handle->data;
    this->total_size = 0;

    while(((fat_elf_header_t *)tail)->magic == 0xBA55ED50) {
        LOG_VERBOSE("Processing fat binary section at %p\n", tail);
        fat_elf_header_t *header = (fat_elf_header_t *)tail;
        elf_files.push_back(std::pair(header, std::vector<fat_elf_item_t *>()));
        auto &curr_vec = elf_files.back().second;

        uint8_t *fatbin_head = (uint8_t *)(((char *)header) + header->header_size);
        uint8_t *fatbin_tail = (uint8_t *)(fatbin_head + header->size);

        this->total_size += header->header_size + header->size;
        
        while (fatbin_head < fatbin_tail) {
            LOG_VERBOSE("Processing fat binary item at %p (end: %p)\n", fatbin_head, fatbin_tail);
#ifdef VERBOSE
            dump_fatbin_header((fat_elf_item_t *)fatbin_head);
#endif
            fat_elf_item_t *item = (fat_elf_item_t *)fatbin_head;
            curr_vec.push_back(item);

            Elf64_Ehdr *elf_file = (Elf64_Ehdr *)(fatbin_head + item->header_size);

            fatbin_head += item->header_size + item->size;
        }

        // Move to the next item in the fat binary
        tail += header->header_size + header->size;
        LOG_VERBOSE("Tail moved to %p\n", tail);
    }

    // this->dump("fatbin_dump_original.bin");
}

void FatBinaryParser::instrument(const Allocation *code_allocation) {
    if (!code_allocation) {
        fprintf(stderr, "Error: Invalid code allocation.\n");
        exit(EXIT_FAILURE);
    }

    LOG("Instrumenting fat binary sections with code allocation: %p\n", code_allocation->base);

    int computeCapabilityMajor, computeCapabilityMinor, device_id;
    cudaGetDevice(&device_id);
    LOG_VERBOSE("\tCurrent CUDA device ID: %d\n", device_id);
    cudaDeviceGetAttribute(&computeCapabilityMajor, cudaDevAttrComputeCapabilityMajor, device_id);
    LOG_VERBOSE("\tCurrent device compute capability major: %d\n", computeCapabilityMajor);
    cudaDeviceGetAttribute(&computeCapabilityMinor, cudaDevAttrComputeCapabilityMinor, device_id);
    LOG_VERBOSE("\tCurrent device compute capability minor: %d\n", computeCapabilityMinor);

    // We support compute capability 8.0 and above, which corresponds to Ampere architecture and above.
    if(computeCapabilityMajor < 8) {
        LOG_ERROR("Unsupported compute capability %d.%d, skipping instrumentation\n", computeCapabilityMajor, computeCapabilityMinor);
        return;
    }

    // For each fat_elf_header_t, we have a list of fat_elf_item_t and its contents
    // [
    //   fat_elf_header: [
    //     (uint8_t *data, size_t size, bool is_dynamic_memory)
    //   ]
    // ]
    std::vector<std::pair<fat_elf_header_t *, std::vector<std::tuple<uint8_t *, size_t, bool>>>> instrumented_sections;

    for(size_t i = 0; i < elf_files.size(); i++) {
        auto &fat_header = elf_files[i].first;
        auto &fat_items = elf_files[i].second;

        instrumented_sections.push_back(std::make_pair(fat_header, std::vector<std::tuple<uint8_t *, size_t, bool>>()));
        auto &instrumented_items = instrumented_sections.back().second;

        LOG_VERBOSE("Processing ELF files from fat binary section %zu\n", i);

        for (auto &item : fat_items) {
            bool is_supported = true;

            LOG_VERBOSE(" - Found ELF item of type %u at %p\n", item->type, item);
            if(item->obj_name_len > 0) {
                LOG_VERBOSE("   Source name: %.*s\n", item->obj_name_len, item->src_name);
            }

            if(item->compute_version > (computeCapabilityMajor * 10 + computeCapabilityMinor)) {
                LOG_VERBOSE("Item compute version %u is higher than device compute capability %d.%d\n",
                            item->compute_version, computeCapabilityMajor, computeCapabilityMinor);
                is_supported = false;
            } else if (item->compute_version / 10 < computeCapabilityMajor) {
                LOG_VERBOSE("\tItem compute major version %u is lower than device compute capability major %d\n",
                            item->compute_version / 10, computeCapabilityMajor);
                is_supported = false;
            } else {
                LOG_VERBOSE("\tItem compute version %u is compatible with device compute capability %d.%d\n",
                            item->compute_version, computeCapabilityMajor, computeCapabilityMinor);
            }

            if(is_supported && (item->flags & FATBIN_FLAG_COMPRESS)) {
                LOG_VERBOSE("   Item is compressed (compressed size: %u, uncompressed size: %llu)\n",
                            item->compressed_size, item->uncompressed_size);
                if (decompress_fatbin(&item) < 0) {
                    LOG_ERROR("Failed to decompress fatbin item\n");
                    is_supported = false;
                }
            }
            
            if (is_supported) {
                switch (item->type) {
                    case FAT_BIN_ITEM_TYPE_CUBIN:
                        LOG_VERBOSE("\tItem is a CUBIN\n");
                        // Nothing to do, item is ready for instrumentation
                        is_supported = true;
                        break;
                    case FAT_BIN_ITEM_TYPE_PTX:
                        LOG_VERBOSE("\tItem is PTX, trying to compile to CUBIN\n");
                        // We need to compile PTX to CUBIN first
                        if(compile_ptx_to_cubin(&item, computeCapabilityMajor, computeCapabilityMinor) != 0) {
                            LOG_ERROR("Failed to compile PTX to CUBIN\n");
                            is_supported = false;
                        } else {
                            LOG_VERBOSE("\tSuccessfully compiled PTX to CUBIN\n");
                            is_supported = true;
                        }
                        break;
                    default:
                        LOG_VERBOSE("\tItem is of unknown type %u\n", item->type);
                        is_supported = false;
                        break;
                }
            }

            // TODO memory management (I am not freeing dynamically allocated memory in this step)

            if (is_supported) {

                LOG_VERBOSE("\tInstrumenting item of type %u with compute version %u\n", item->type, item->compute_version);

                try {
                    cuELF cuelf = cuELF((uint8_t *)item + item->header_size);
                    cuelf.instrument_sections(code_allocation->base, code_allocation->length);

                    size_t new_size = cuelf.get_elf_size();
                    LOG_VERBOSE("\tNew ELF file size: 0x%zx\n", new_size);

                    uint8_t *data = new uint8_t[item->header_size + new_size];
                    memcpy(data, item, item->header_size);
                    ((fat_elf_item_t *)data)->size = new_size;
                    cuelf.export_bytes(data + item->header_size);

                    instrumented_items.push_back(std::make_tuple(
                        data,
                        new_size + item->header_size,
                        true
                    ));
                } catch (const std::exception &e) {
                    LOG_ERROR("Failed to instrument item of type %u: %s\n", item->type, e.what());

                    // If instrumentation fails, we still want to keep the item for further processing
                    instrumented_items.push_back(std::make_tuple(
                        (uint8_t *)item,
                        item->header_size + item->size,
                        false
                    ));
                }

            } else {
                // If the item is not supported, we still want to keep it for further processing
                instrumented_items.push_back(std::make_tuple(
                    (uint8_t *)item,
                    item->header_size + item->size,
                    false
                ));
            }
        }
    }

    size_t total_size = 0;
    for (const auto &section : instrumented_sections) {
        total_size += section.first->header_size;
        for (const auto &item : section.second) {
            total_size += std::get<1>(item);
        }
    }

    if (this->total_size != total_size) {
        LOG_VERBOSE("Total size changed: %zu, %zu\n", this->total_size, total_size);
    }

    uint8_t *new_data = new uint8_t[total_size];
    uint8_t *ptr = new_data;

    for (const auto &section : instrumented_sections) {
        fat_elf_header_t *header = section.first;
        memcpy(ptr, header, header->header_size);
        header = (fat_elf_header_t *)ptr;
        ptr += header->header_size;
        uint8_t *data_start = ptr;

        for (const auto &item : section.second) {
            memcpy(ptr, std::get<0>(item), std::get<1>(item));
            ptr += std::get<1>(item);
            if (std::get<2>(item)) {
                // If the item is dynamically allocated, we need to free it
                LOG("About to free %zu bytes\n", std::get<1>(item));
                delete[] std::get<0>(item);
            }
        }

        header->size = ptr - data_start;
        LOG_VERBOSE("Wrote section with header size %u and total size %zu\n", header->header_size, header->size);
    }

    handle->data = (long long unsigned int *)new_data;
    // this->dump("/tmp/fatbin_dump.bin");
}

void FatBinaryParser::dump(const char *filename) {
    if(this->total_size == 0) {
        LOG_ERROR("No data to dump, total size is 0.\n");
        return;
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        LOG_ERROR("Failed to open file for dumping: %s\n", filename);
        return;
    }

    fwrite(this->handle->data, 1, this->total_size, file);
    fclose(file);

    LOG_VERBOSE("Dumped fatbin to %s\n", filename);
}

static int flag_to_str(char** str, uint64_t flag)
{
    return asprintf(str, "64Bit: %s, Debug: %s, Linux: %s, Compress: %s",
        (flag & FATBIN_FLAG_64BIT) ? "yes" : "no",
        (flag & FATBIN_FLAG_DEBUG) ? "yes" : "no",
        (flag & FATBIN_FLAG_LINUX) ? "yes" : "no",
        (flag & FATBIN_FLAG_COMPRESS) ? "yes" : "no");
}

static void dump_fatbin_header(fat_elf_item_t *header) {
    char *flag_str = NULL;
    flag_to_str(&flag_str, header->flags);
    LOG("Fatbin Item Header:\n");
    LOG(" - Type: %s\n", (header->type == FAT_BIN_ITEM_TYPE_PTX) ? "PTX" :
                         (header->type == FAT_BIN_ITEM_TYPE_CUBIN) ? "CUBIN" : "Unknown");
    LOG(" - Header Size: %u\n", header->header_size);
    LOG(" - Size: %llu\n", header->size);
    LOG(" - Compressed Size: %u\n", header->compressed_size);
    LOG(" - Compute Version: %u\n", header->compute_version);
    LOG(" - Major.Minor: %u.%u\n", header->major, header->minor);
    LOG(" - Object Name Offset: %u\n", header->obj_name_offset);
    LOG(" - Object Name Length: %u\n", header->obj_name_len);
    LOG(" - Flags: %s\n", flag_str ? flag_str : "N/A");
    LOG(" - Uncompressed Size: %llu\n", header->uncompressed_size);
    if (header->obj_name_len > 0) {
        LOG(" - Source Name: %.*s\n", header->obj_name_len, header->src_name);
    }
    free(flag_str);
}

int decompress_fatbin(fat_elf_item_t **pitem) {
    LOG_VERBOSE("\tDecompressing fatbin item...\n");
    fat_elf_item_t *item = *pitem;
    size_t decompressed_size = item->header_size + item->uncompressed_size + 8; // +8 for possible padding
    uint8_t *decompressed_data = new uint8_t[decompressed_size];

    int64_t decompress_ret = decompress_fatbin(
        item,
        decompressed_data,
        &decompressed_size
    );

    if (decompress_ret < 0) {
        LOG_ERROR("Decompression failed for fatbin item at %p\n", item);
        delete [] decompressed_data;
        return -1;
    } else {
        LOG_VERBOSE("\tDecompression successful, new size: %zu\n", decompressed_size);
        LOG_VERBOSE("\tOriginal size: %zu, Compressed size: %u, Uncompressed size: %llu\n",
                    item->size, item->compressed_size, item->uncompressed_size);
        fat_elf_item_t *decompressed_item = (fat_elf_item_t *)decompressed_data;
        *pitem = (fat_elf_item_t *)decompressed_data;
        return 0;
    }
}

#define NVPTXCOMPILER_SAFE_CALL(x)                                       \
    do {                                                                 \
        nvPTXCompileResult result = x;                                   \
        if (result != NVPTXCOMPILE_SUCCESS) {                            \
            LOG_ERROR("error: %s failed with error code %d\n", #x, result); \
            return -1;                                                   \
        }                                                                \
    } while(0)

int compile_ptx_to_cubin(fat_elf_item_t **pitem, uint32_t computeCapabilityMajor, uint32_t computeCapabilityMinor) {
    LOG_VERBOSE("\tCompiling PTX to CUBIN...\n");
    fat_elf_item_t *item = *pitem;
    
    const char *ptx_code = (const char *)item + item->header_size;
    size_t ptx_size = item->size;

    // LOG_VERBOSE("PTX CODE:\n%.*s\n", (int)ptx_size, ptx_code);

    char gpu_name[64];
    snprintf(gpu_name, sizeof(gpu_name), "--gpu-name=sm_%d%d", computeCapabilityMajor, computeCapabilityMinor);
    LOG_VERBOSE("\tCurrent device GPU name: %s\n", gpu_name);

    // TODO: debug this part
    // size_t options_size;
    // char *fatbin_compile_options = get_fatbin_compile_options(item, &options_size);
    // if (options_size > 0) {
    //     LOG_VERBOSE("\tFound compile options in the fatbin item: %s\n", fatbin_compile_options);
    // } else {
    //     LOG_VERBOSE("\tNo compile options found in the fatbin item, using default options\n");
    // }
    
    nvPTXCompilerHandle compiler;
    NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerCreate(&compiler, ptx_size, ptx_code));
    // const char* compile_options[] = { gpu_name, "--verbose", options_size > 0 ? fatbin_compile_options : "" };
    const char* compile_options[] = { gpu_name, "--verbose" };

    nvPTXCompileResult status = nvPTXCompilerCompile(compiler,
                                  2,                 /* numCompileOptions */
                                  compile_options);  /* compileOptions */

    if (status != NVPTXCOMPILE_SUCCESS) {
        size_t errorSize;
        char* errorLog;
        NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerGetErrorLogSize(compiler, &errorSize));

        if (errorSize != 0) {
            errorLog = (char*)malloc(errorSize+1);
            NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerGetErrorLog(compiler, errorLog));
            LOG_ERROR("Error log: %s\n", errorLog);
            free(errorLog);
        }
        return -1;
    }

    size_t elfSize;
    char* elf;

    NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerGetCompiledProgramSize(compiler, &elfSize));

    elf = (char*) malloc(elfSize);
    NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerGetCompiledProgram(compiler, (void*)elf));
    NVPTXCOMPILER_SAFE_CALL(nvPTXCompilerDestroy(&compiler));

    size_t new_size = item->header_size + elfSize;
    uint8_t *new_data = new uint8_t[new_size];
    memcpy(new_data, item, item->header_size);
    fat_elf_item_t *new_item = (fat_elf_item_t *)new_data;
    new_item->type = FAT_BIN_ITEM_TYPE_CUBIN;
    new_item->size = elfSize;
    memcpy(new_data + item->header_size, elf, elfSize);

    free(elf);

    *pitem = new_item;

    return 0;
}

char *get_fatbin_compile_options(fat_elf_item_t *item, size_t *options_size) {
    if (item->type != FAT_BIN_ITEM_TYPE_PTX) {
        LOG_ERROR("Item is not PTX, cannot get compile options\n");
        return NULL;
    }

    uintptr_t obj_name_end = (uintptr_t)item + item->obj_name_offset + item->obj_name_len;
    obj_name_end = (obj_name_end + 7) & ~0x7; // Align to 8 bytes
    
    uint32_t *compile_options_offset_ptr = (uint32_t *)obj_name_end;
    uint32_t compile_options_offset = *compile_options_offset_ptr;

    uint32_t *compile_options_len_ptr = compile_options_offset_ptr + 1;
    uint32_t compile_options_len = *compile_options_len_ptr;
    LOG_VERBOSE("\tCompile options len pointer: %p, content: %u\n", compile_options_len_ptr, compile_options_len);

    if (compile_options_offset == 0 || compile_options_len == 0) {
        LOG_VERBOSE("No compile options found in the fatbin item\n");
        *options_size = 0;
        return NULL;
    }

    char *compile_options = (char *)item + compile_options_offset;
    *options_size = compile_options_len;

    return compile_options;
}