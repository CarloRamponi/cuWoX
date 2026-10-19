#include <stdio.h>
#include <cuda_runtime.h>
#include <stdint.h>
#include <allocations.h>

struct addresses {
    uint64_t cudart_code;
    uint64_t kernel_code;
};

__device__ __attribute_noinline__ int deviceFunc(int a, int b) {
    uint64_t c = a + b;
    printf("Inside deviceFunc: a=%d, b=%d, c=%lu\n", a, b, c);
    return c;
}

__global__ void kernel(struct addresses *addrs) {
    uint32_t idx = deviceFunc(threadIdx.x, threadIdx.y);

    if (threadIdx.x == 0) {
        // addrs->cudart_code = (uint64_t)deviceFunc;
        addrs->kernel_code = (uint64_t)deviceFunc;
    }
}

int main() {
    struct addresses h_addresses;
    struct addresses *d_addresses;
    cudaMalloc(&d_addresses, sizeof(struct addresses));
    
    kernel<<<1, 1>>>(d_addresses);
    cudaError_t err = cudaDeviceSynchronize();

    if (err != cudaSuccess) {
        fprintf(stderr, "CUDA error: %s\n", cudaGetErrorString(err));
        return -1;
    }

    cudaMemcpy(&h_addresses, d_addresses, sizeof(struct addresses), cudaMemcpyDeviceToHost);
    // printf("CUDA Runtime code address:\t0x%lx\n", h_addresses.cudart_code);
    printf("Kernel code address:\t\t0x%lx\n", h_addresses.kernel_code);

    // int cudart_code_id = find_allocation_id(h_addresses.cudart_code);
    int kernel_code_id = find_allocation_id(h_addresses.kernel_code);

    // printf("CUDA Runtime code allocation ID:\t%d\n", cudart_code_id);
    printf("Kernel code allocation ID:\t\t%d\n", kernel_code_id);
    // Assuming the runtime code is allocated two pages before the kernel code:
    printf("CuDA Runtime code allocation ID (assumed):\t%d\n", kernel_code_id - 2);
    return 0;
}
