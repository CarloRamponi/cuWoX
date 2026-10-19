#include <stdio.h>
#include <cuda_runtime.h>
#include <stdint.h>

__device__ __noinline__ void injected_function() {
    printf("Injected function called\n");
    // asm("trap;");
    while(1);
}

__device__ __noinline__ int deviceFunc(uint64_t *a) {
    // get the value of stack pointer in asm
    uint32_t *sp;
    asm("mov.u64 %0, %%SP;" : "=l"(sp));

    if(threadIdx.x == 0) {
        printf("SP = %p\n", sp);
        for(int i = 0; i < 0xF; i++) {
            printf("Stack[+%02x] (%p) = 0x%08x\t64-bit: 0x%016lx\n", i * 4, &sp[i], sp[i], (uint64_t)(sp[i+1]) << 32 | sp[i]);
        }
    }

    // overwrite the return address
    sp[0x20 / 4] = (uint32_t)a;
    sp[0x24 / 4] = (uint32_t)((uint64_t)a >> 32);

    // asm("trap;");
    return 0;
}

__global__ void kernel(uint64_t *a) {

    if(threadIdx.x == 0) {
        memcpy(a, &injected_function, 128 * sizeof(uint64_t));
        printf("Injected function copied\n");
        printf("a = %p\n", a);
        for(int i = 0; i < 32; i++) {
            printf("a[%d] = 0x%016lx\n", i, a[i]);
        }
    }

    __syncthreads();

    deviceFunc(a);
}

int main() {
    uint64_t *d_a;
    cudaMalloc(&d_a, 128 * sizeof(uint64_t));
    kernel<<<1, 32>>>(d_a);
    cudaError_t err = cudaDeviceSynchronize();

    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString(err));
        return -1;
    }

    return 0;
}
