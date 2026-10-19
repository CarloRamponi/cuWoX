#include <cuda_runtime.h>
#include <stdio.h>

#define CHECK_CUDA_ERROR(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"Error: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__device__ __noinline__ int mul_imm(int a) {
    return a * 2;
}

__global__ void multiply(int * destination, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        destination[idx] = mul_imm(destination[idx]);
    }
}

__device__ __noinline__ int obfuscate(int a) {
    return a + 20;
}

__global__ void copy_with_offset_kernel(const int* source, int* destination, int offset, int size) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < size) {
        destination[idx + offset] = obfuscate(source[idx]);
    }
}

__global__ void get_addr() {
    if (threadIdx.x == 0 && blockIdx.x == 0) {
        printf("\tmul_imm device: %p\n", mul_imm);
    }
}

int main() {
    int source_size = 0;
    int offset = 0;
    const int destination_size = 200;

    int *h_source, *h_destination;
    int *d_dummy, *d_source, *d_destination;

    /* ALLOCATE DUMMY HEAP */
    CHECK_CUDA_ERROR(cudaMalloc(&d_dummy, 20 * sizeof(int)));

    while (true) {
        printf("[i] Insert input size: ");
        scanf("%d", &source_size);
        
        h_source = (int *) malloc(source_size * sizeof(int));
        h_destination = (int *) malloc(destination_size * sizeof(int));
        
        printf("[i] Insert the numbers to obfuscate (MAX %d):\n", source_size);
        for(int i = 0; i < source_size; i++) {
            scanf("%d", &h_source[i]);
        }

        CHECK_CUDA_ERROR(cudaMalloc(&d_source, source_size * sizeof(int)));
        CHECK_CUDA_ERROR(cudaMalloc(&d_destination, destination_size * sizeof(int)));

        printf("[i] Useful addresses:\n");
        get_addr<<<1,1>>>();
        printf("\td_destination:  %p\n", d_destination);
        CHECK_CUDA_ERROR(cudaDeviceSynchronize());

        printf("[i] Insert desired offset: ");
        scanf("%d", &offset);

        cudaMemcpy(d_source, h_source, source_size * sizeof(int), cudaMemcpyHostToDevice);

        int threadsPerBlock = 256;
        int blocksPerGrid = (source_size + threadsPerBlock - 1) / threadsPerBlock;

        copy_with_offset_kernel<<<blocksPerGrid, threadsPerBlock>>>(d_source, d_destination, offset, source_size);
        CHECK_CUDA_ERROR(cudaDeviceSynchronize());

        CHECK_CUDA_ERROR(cudaMemcpy(h_destination, d_destination, destination_size * sizeof(int), cudaMemcpyDeviceToHost));
        printf("[i] Obfuscated array on host with offset %d\n", offset);
        for (int i = 0; i < destination_size; ++i) {
            printf("%d ", h_destination[i]);
        }
        printf("\n");

        printf("Multiplying obfuscated data!\n");
        multiply<<<blocksPerGrid,threadsPerBlock>>>(d_destination, source_size+offset);
        
        CHECK_CUDA_ERROR(cudaDeviceSynchronize());

        CHECK_CUDA_ERROR(cudaMemcpy(h_destination, d_destination, destination_size * sizeof(int), cudaMemcpyDeviceToHost));
        printf("[i] Multiplied array on host with offset %d\n", offset);
        for (int i = 0; i < destination_size; ++i) {
            printf("%d ", h_destination[i]);
        }
        printf("\n");
        
        CHECK_CUDA_ERROR(cudaMemset(d_destination, 0, destination_size * sizeof(int)));

        cudaFree(d_source);
        cudaFree(d_destination);
        free(h_source);
        free(h_destination);

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

    cudaFree(d_dummy);

    return 0;
}