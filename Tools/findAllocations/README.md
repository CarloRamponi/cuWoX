# Find Allocation index of kernel code

This tool tries to automatically identify the allocation index for code pages.

### Build

You should be able to build the tool with a simple `make` command.
This should generate two files: `app` and `liballocations.so`, both are needed.

### Run

To correctly run the file, the loader needs to find our `liballocation.so` shared library, thus:

```
LD_LIBRARY_PATH=. ./app
```

This should print something like:

```
Kernel code address:		        0x7f2ddb289000
Kernel code allocation ID:		    18
CuDA Runtime code allocation ID:	16
```

## Then?

Once you have the allocation indexes, you should update the definitions in the header file at [cuWX/src/runtime/allocations.h](../../cuWX/src/runtime/allocations.h)

```c++
#define CUDART_CODE_ALLOCATION_ID 16 // This is the CUDA Runtime code allocation ID
#define KERNEL_CODE_ALLOCATION_ID 18 // This is the Kernel code allocation ID
#define CONTEXT_INIT_ALLOCATION_MIN 18 // This is the max of the previous two values
```