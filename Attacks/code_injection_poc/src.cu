#include <stdio.h>
#include <cuda_runtime.h>
#include <stdint.h>

__device__ __noinline__ unsigned int optmized_sum(unsigned int a, unsigned int b) {
    return a + b;
}

__device__ __noinline__ unsigned int sum_array(int *a) {
    int tmp[256];
    int i;

    for (i = 0; a[i] != -1; i++) {
        tmp[i] = a[i];
    }

    unsigned int sum = 0;

    for (int j = 0; j <= i; j++) {
        sum = optmized_sum(sum, (unsigned int) tmp[j]);
    }
    
    return sum;
}

__global__ void kernel(int *vec, unsigned int *res) {
    *res = sum_array(vec);
}

bool checkResult(int *vec, int n, int res) {
    int sum = 0;
    for(int i = 0; i < n && vec[i] != -1; i++) {
        sum += vec[i];
    }
    return sum == res;
}

int main() {
    constexpr int N = 1024;
    int vec[N];
    unsigned int res, *d_res;
    int *d_vec;

    cudaMalloc(&d_vec, N * sizeof(int));
    cudaMalloc(&d_res, sizeof(unsigned int));

    while (true) {

        printf("Insert the numbers to sum, ending with a -1 (MAX %d):\n", N);
        for(int i = 0; i < 1024; i++) {
            scanf("%d", &vec[i]);
            if(vec[i] == -1)
                break;
        }
    
        cudaMemcpy(d_vec, vec, N * sizeof(int), cudaMemcpyHostToDevice);
        kernel<<<1, 1>>>(d_vec, d_res);
        
        cudaError_t err = cudaDeviceSynchronize();
        if (err != cudaSuccess) {
            printf("CUDA error: %s\n", cudaGetErrorString(err));
            return 1;
        }
        
        cudaMemcpy(&res, d_res, sizeof(int), cudaMemcpyDeviceToHost);
        
        if (checkResult(vec, N, res)) {
            printf("TEST PASSED\n   Result: %u - 0x%016x\n", res, res);
        } else {
            printf("TEST FAILED\n   Result: %u - 0x%016x\n", res, res);
        }
    
        printf("Do you want to exit? (y/n): ");
        char answer;
        do {
            answer = getchar();
        } while (answer == '\n');

        if (answer == 'y' || answer == 'Y') {
            printf("Exiting...\n");
            break;
        }

    }
    return 0;
}