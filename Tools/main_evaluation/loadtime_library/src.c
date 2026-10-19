#include <time.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdio.h>

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
}

cudaError_t handle_cudaLaunchKernel(const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream) {
    evaluation_load_time_finish(NULL);
    exit(0);
}

cudaError_t cudaLaunchKernel ( const void* func, dim3 gridDim, dim3 blockDim, void** args, size_t sharedMem, cudaStream_t stream ) {
    return handle_cudaLaunchKernel(func, gridDim, blockDim, args, sharedMem, stream);
}