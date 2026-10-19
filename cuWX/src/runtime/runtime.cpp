#include <runtime.h>
#include <fatbin.h>
#include <log.h>
#include <cstring>
#include <unordered_set>
#include <vector>

std::vector<__fatBinC_Wrapper_t *> *global_fatbin_handle = NULL;
std::unordered_set<uint64_t> *module_loaded = NULL;

#ifdef LOADTIME_EVAL
#include <time.h>
#include <signal.h>
struct timespec tstart_load;

__attribute__((constructor)) void init_timer() {
    clock_gettime(CLOCK_MONOTONIC, &tstart_load);
    printf("[cuDEP]: Loadtime evaluation timer initialized\n");
}

void evaluation_load_time_finish(void *userData) {
    struct timespec tend_load;
    clock_gettime(CLOCK_MONOTONIC, &tend_load);
    printf("[cuDEP]: Loadtime evaluation time: %f milliseconds\n",
           ((double)tend_load.tv_sec * 1000 + 1.0e-6*tend_load.tv_nsec) -
           ((double)tstart_load.tv_sec * 1000 + 1.0e-6*tstart_load.tv_nsec));
    fflush(stdout);
    fflush(stderr);
    // exit(0);
    kill(getpid(), SIGKILL);
}

#endif


void runtime_kernel_launch_callback(void* userData) {
    bool enable = (bool)userData;
    // Once the kernel has finished executing, we can set the permissions back to read-write
    set_ro_permissions(enable);
}

void allocation_callback(Allocation * allocations, size_t n) {
    if(global_fatbin_handle) {
        LOG("Patching fat binary data after context initialization\n");

        const Allocation *code_alloc = get_kernel_code_allocation();
        if(!code_alloc) {
            LOG("No allocation found for code, cannot patch fat binary.\n");
            return;
        }

        for (auto &handle : *global_fatbin_handle) {
            FatBinaryParser fatbin = FatBinaryParser(handle);
            fatbin.instrument(code_alloc);
        }

    } else {
        LOG("No global fat binary handle found, skipping patching.\n");
        return;
    }
}

CUresult handle_cuModuleLoad(CUmodule *module, const char *fname) {
    const Allocation* kernel_alloc = get_kernel_code_allocation();

    if (kernel_alloc) {
        try {
            cuELF *elf = cuELF::from_file(fname);
            elf->instrument_sections(kernel_alloc->base, kernel_alloc->length);
            size_t size = elf->get_elf_size();
            uint8_t *data = new uint8_t[size];
            elf->export_bytes(data);
            delete elf;

            LOG_VERBOSE("cuModuleLoad: Successfully patched module (in memory): %s\n", fname);

            CUresult ret = real_cuModuleLoadData(module, data);
            if (ret != CUDA_SUCCESS) {
                LOG_ERROR("cuModuleLoad: Failed to load module data: %d\n", ret);
            }
            delete[] data;

            return ret;
        } catch (const std::exception &e) {
            LOG_ERROR("cuModuleLoad: Failed to patch module: %s\n", e.what());
        }
    } else {
        LOG_ERROR("cuModuleLoad: No kernel code allocation found\n");
    }

    // Fallback to loading the original module
    CUresult ret = real_cuModuleLoad(module, fname);

    return ret;
}

CUresult handle_cuModuleLoadData(CUmodule *module, const void* image) {
    const Allocation* kernel_alloc = get_kernel_code_allocation();

    if (kernel_alloc) {

        // For the moment, this only works with CUBIN files.
        // If a FATBIN is provided, it will not work.

        try {
            cuELF *elf = new cuELF((const uint8_t *)image);
            elf->instrument_sections(kernel_alloc->base, kernel_alloc->length);
            size_t size = elf->get_elf_size();
            uint8_t *data = new uint8_t[size];
            elf->export_bytes(data);
            delete elf;

            LOG_VERBOSE("cuModuleLoadData: Successfully patched module (in memory)\n");

            CUresult ret = real_cuModuleLoadData(module, data);
            if (ret != CUDA_SUCCESS) {
                LOG_ERROR("cuModuleLoadData: Failed to load module data: %d\n", ret);
            }
            delete[] data;

            return ret;
        } catch (const std::exception &e) {
            LOG_ERROR("cuModuleLoadData: Failed to patch module: %s\n", e.what());
        }
    } else {
        LOG_ERROR("cuModuleLoadData: No kernel code allocation found\n");
    }

    // Fallback to loading the original module
    CUresult ret = real_cuModuleLoadData(module, image);

    return ret;
}

CUresult handle_cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues) {
    const Allocation* kernel_alloc = get_kernel_code_allocation();

    if (kernel_alloc) {

        // For the moment, this only works with CUBIN files.
        // If a FATBIN is provided, it will not work.

        try {
            cuELF *elf = new cuELF((const uint8_t *)image);
            elf->instrument_sections(kernel_alloc->base, kernel_alloc->length);
            size_t size = elf->get_elf_size();
            uint8_t *data = new uint8_t[size];
            elf->export_bytes(data);
            delete elf;

            LOG_VERBOSE("cuModuleLoadData: Successfully patched module (in memory)\n");

            CUresult ret = real_cuModuleLoadDataEx(module, data, numOptions, options, optionValues);
            if (ret != CUDA_SUCCESS) {
                LOG_ERROR("cuModuleLoadData: Failed to load module data: %d\n", ret);
            }
            delete[] data;

            return ret;
        } catch (const std::exception &e) {
            LOG_ERROR("cuModuleLoadData: Failed to patch module: %s\n", e.what());
        }
    } else {
        LOG_ERROR("cuModuleLoadData: No kernel code allocation found\n");
    }

    // Fallback to loading the original module
    CUresult ret = real_cuModuleLoadDataEx(module, image, numOptions, options, optionValues);

    return ret;
}

CUresult handle_cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra) {
    LOG("Launching kernel (driver APIs)\n");

    cuLaunchHostFunc(hStream, runtime_kernel_launch_callback, (void *)true);
    CUresult ret = real_cuLaunchKernel(f, gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY, blockDimZ, sharedMemBytes, hStream, kernelParams, extra);
    cuLaunchHostFunc(hStream, runtime_kernel_launch_callback, (void *)false);
    return ret;
}

cudaError_t handle_cudaLaunchKernel(const void *func, dim3 gridDim, dim3 blockDim, void **args, size_t sharedMem, cudaStream_t stream) {
    LOG("Launching kernel (runtime APIs)\n");

    // Force cuda to load the module in GPU memory by querying
    // function attributes so that we can set the read-only bit.
    // TODO: check for the module inside the opaque structure
    if (module_loaded == NULL) {
        module_loaded = new std::unordered_set<uint64_t>();
    }

    if(!module_loaded->contains((uint64_t)func)) {
        LOG_VERBOSE("Loading module for function: %p\n", func);
        cudaFuncAttributes attr;
        cudaFuncGetAttributes(&attr, func);
        module_loaded->emplace((uint64_t)func);
    }

    cudaLaunchHostFunc(stream, runtime_kernel_launch_callback, (void *)true);

#ifdef LOADTIME_EVAL
    cudaLaunchHostFunc(stream, evaluation_load_time_finish, NULL);
#endif

    cudaError_t ret = real_cudaLaunchKernel(func, gridDim, blockDim, args, sharedMem, stream);

    cudaLaunchHostFunc(stream, runtime_kernel_launch_callback, (void *)false);
    return ret;
}

void *handle___cudaRegisterFatBinary(__fatBinC_Wrapper_t *handle) {
    LOG("Registering fat binary\n");

    if (!global_fatbin_handle) {
        global_fatbin_handle = new std::vector<__fatBinC_Wrapper_t *>();
    }

    LOG_VERBOSE("Allocating memory for the fatbinary and copying data\n");
    __fatBinC_Wrapper_t * new_handle = (__fatBinC_Wrapper_t *)malloc(sizeof(__fatBinC_Wrapper_t));
    global_fatbin_handle->push_back(new_handle);
    memcpy(new_handle, handle, sizeof(__fatBinC_Wrapper_t));

    add_allocation_callback(allocation_callback);

    void *ret = real__cudaRegisterFatBinary(new_handle);
    return ret;
}

CUresult handle_cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin) {
    LOG("Loading fat binary module\n");
    
    LOG("Raising breakpoint for memory inspection\n");
    // asm("int $3");
    
    // TODO: handle fatbin here
    return real_cuModuleLoadFatBinary(module, fatCubin);
}
