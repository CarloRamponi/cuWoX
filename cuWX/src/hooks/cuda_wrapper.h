#ifndef CUDA_WRAPPER_H
#define CUDA_WRAPPER_H

#include <cuda.h>
#include <cuda_runtime_api.h>
#include <fatbinary_section.h>

#ifdef __cplusplus
extern "C" {
#endif

// This function is called by NVBIT, in case we want to also support statically linked kernels
// void kernelLaunchCallback(CUcontext ctx, CUfunction func, const char *name, void *params);


/* Handle runtime API calls */

void * __cudaRegisterFatBinary(__fatBinC_Wrapper_t * handle);
void * real__cudaRegisterFatBinary(__fatBinC_Wrapper_t * handle);

cudaError_t cudaLaunchKernel ( const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream );
cudaError_t real_cudaLaunchKernel ( const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream );


/* Handle driver API calls */

CUresult cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra);
CUresult real_cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra);

CUresult cuModuleLoad(CUmodule *module, const char *fname);
CUresult real_cuModuleLoad(CUmodule *module, const char *fname);

CUresult cuModuleLoadData(CUmodule* module, const void* image);
CUresult real_cuModuleLoadData(CUmodule *module, const void* image);

CUresult cuModuleLoadDataEx ( CUmodule* module, const void* image, unsigned int  numOptions, CUjit_option* options, void** optionValues );
CUresult real_cuModuleLoadDataEx ( CUmodule* module, const void* image, unsigned int  numOptions, CUjit_option* options, void** optionValues );

CUresult cuModuleLoadFatBinary ( CUmodule* module, const void* fatCubin );
CUresult real_cuModuleLoadFatBinary ( CUmodule* module, const void* fatCubin );

#ifdef __cplusplus
}
#endif

#endif