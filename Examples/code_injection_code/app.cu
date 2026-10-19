#include <stdio.h>
#include <cuda_runtime.h>
#include <stdint.h>

__device__ int injected_function() {
    printf("Injected function called\n");
    return 42;
}

__device__ int deviceFunc(int a, int b) {
    uint64_t c = a + b;
    return c;
}

__global__ void kernel(int *a, int *b, int *c) {
    uint32_t idx = threadIdx.x;

    if(threadIdx.x == 0) {
        memcpy(deviceFunc, &injected_function, 128 * sizeof(uint64_t));
    }

    c[idx] = deviceFunc(a[idx], b[idx]);
}

int main() {
    int a[32], b[32], c[32];
    int *d_a, *d_b, *d_c;
    cudaMalloc(&d_a, 32 * sizeof(int));
    cudaMalloc(&d_b, 32 * sizeof(int));
    cudaMalloc(&d_c, 32 * sizeof(int));
    for(int i = 0; i < 32; i++) {
        a[i] = i;
        b[i] = 2 * i;
    }
    cudaMemcpy(d_a, a, 32 * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, b, 32 * sizeof(int), cudaMemcpyHostToDevice);
    kernel<<<1, 32>>>(d_a, d_b, d_c);
    cudaError_t err = cudaDeviceSynchronize();

    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString(err));
        return -1;
    }

    cudaMemcpy(c, d_c, 32 * sizeof(int), cudaMemcpyDeviceToHost);
    for(int i = 0; i < 32; i++) {
        printf("c[%d] = %d\n", i, c[i]);
    }
    return 0;
}
