#include <cuda_wrapper.h>
#include <dlfcn.h>
#include <cuda.h>
#include <runtime.h>

struct CUmod_st {
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    char *name;
    void *p4;
    uint32_t p5;
    uint32_t p6;
    void *p7;
    void *p8;
    uint64_t p9[3];
    void *p10;
};

struct CUfunc_st {
    uint32_t p1;
    uint32_t p2;
    char *name;
    uint64_t p3;
    uint64_t p4;
    CUmodule module;
    uint64_t p5[7];
    void *p6;
    CUdeviceptr code;
};

CUresult cuModuleLoad(CUmodule *module, const char *fname) {
    return handle_cuModuleLoad(module, fname);
}

CUresult real_cuModuleLoad(CUmodule *module, const char *fname) {
    static CUresult (*cuModuleLoad)(CUmodule *, const char *) = NULL;
    if (!cuModuleLoad)
    {
        cuModuleLoad = (CUresult (*)(CUmodule *, const char *))dlsym(RTLD_NEXT, "cuModuleLoad");
    }
    return cuModuleLoad(module, fname);
}



CUresult cuModuleLoadData(CUmodule *module, const void *image) {
    return handle_cuModuleLoadData(module, image);
}

CUresult real_cuModuleLoadData(CUmodule *module, const void *image) {
    static CUresult (*cuModuleLoadData)(CUmodule *, const void *) = NULL;
    if (!cuModuleLoadData) {
        cuModuleLoadData = (CUresult (*)(CUmodule *, const void *))dlsym(RTLD_NEXT, "cuModuleLoadData");
    }
    return cuModuleLoadData(module, image);
}



CUresult cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues) {
    return handle_cuModuleLoadDataEx(module, image, numOptions, options, optionValues);
}

CUresult real_cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues)
{
    static CUresult (*cuModuleLoadDataEx)(CUmodule *, const void *, unsigned int, CUjit_option *, void **) = NULL;
    if (!cuModuleLoadDataEx) {
        cuModuleLoadDataEx = (CUresult (*)(CUmodule *, const void *, unsigned int, CUjit_option *, void **))dlsym(RTLD_NEXT, "cuModuleLoadDataEx");
    }
    return cuModuleLoadDataEx(module, image, numOptions, options, optionValues);
}



CUresult cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra) {
    return handle_cuLaunchKernel(f, gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY, blockDimZ, sharedMemBytes, hStream, kernelParams, extra);
}

CUresult real_cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra) {
    static CUresult (*cuLaunchKernel)(CUfunction, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, CUstream, void **, void **) = NULL;
    if (!cuLaunchKernel)
    {
        cuLaunchKernel = (CUresult (*)(CUfunction, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, CUstream, void **, void **))dlsym(RTLD_NEXT, "cuLaunchKernel");
    }
    return cuLaunchKernel(f, gridDimX, gridDimY, gridDimZ, blockDimX, blockDimY, blockDimZ, sharedMemBytes, hStream, kernelParams, extra);
}



cudaError_t cudaLaunchKernel ( const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream ) {
    return handle_cudaLaunchKernel(func, gridDim, blockDim, args, sharedMem, stream);
}

cudaError_t real_cudaLaunchKernel ( const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream ) {
    static cudaError_t (*cudaLaunchKernel)(const void*, dim3, dim3, void**, size_t, cudaStream_t) = NULL;
    if (!cudaLaunchKernel)
    {
        cudaLaunchKernel = (cudaError_t (*)(const void*, dim3, dim3, void**, size_t, cudaStream_t))dlsym(RTLD_NEXT, "cudaLaunchKernel");
    }
    return cudaLaunchKernel(func, gridDim, blockDim, args, sharedMem, stream);
}



void * __cudaRegisterFatBinary(__fatBinC_Wrapper_t * handle) {
    return handle___cudaRegisterFatBinary(handle);
}

void * real__cudaRegisterFatBinary(__fatBinC_Wrapper_t * handle) {
    static void * (*cudaRegisterFatBinary)(__fatBinC_Wrapper_t *) = NULL;
    if (!cudaRegisterFatBinary)
    {
        cudaRegisterFatBinary = (void * (*)(__fatBinC_Wrapper_t *))dlsym(RTLD_NEXT, "__cudaRegisterFatBinary");
    }
    return cudaRegisterFatBinary(handle);
}



CUresult cuModuleLoadFatBinary(CUmodule *module, const void *fatCubin) {
    return handle_cuModuleLoadFatBinary(module, fatCubin);
}

CUresult real_cuModuleLoadFatBinary ( CUmodule* module, const void* fatCubin ) {
    static CUresult (*cuModuleLoadFatBinary)(CUmodule*, const void*) = NULL;
    if (!cuModuleLoadFatBinary)
    {
        cuModuleLoadFatBinary = (CUresult (*)(CUmodule*, const void*))dlsym(RTLD_NEXT, "cuModuleLoadFatBinary");
    }
    return cuModuleLoadFatBinary(module, fatCubin);
}