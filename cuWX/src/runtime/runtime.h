#pragma once

#include <fatbinary_section.h>
#include <cuda_runtime_api.h>
#include <allocations.h>
#include <cuda_wrapper.h>

__fatBinC_Wrapper_t * onCudaRegisterFatBinary(__fatBinC_Wrapper_t * handle);
void runtime_kernel_launch_callback(void* userData);
void allocation_callback(Allocation * allocations, size_t n);

/* Handle driver API calls */
CUresult handle_cuModuleLoad(CUmodule *module, const char *fname);
CUresult handle_cuModuleLoadData(CUmodule *module, const void* image);
CUresult handle_cuModuleLoadDataEx(CUmodule *module, const void *image, unsigned int numOptions, CUjit_option *options, void **optionValues);
CUresult handle_cuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void **kernelParams, void **extra);
CUresult handle_cuModuleLoadFatBinary ( CUmodule* module, const void* fatCubin );

/* Handle runtime API calls */
cudaError_t handle_cudaLaunchKernel ( const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream );
void * handle___cudaRegisterFatBinary(__fatBinC_Wrapper_t * handle);