# cuWX

This repository contains cuWX: an implementation of the W^X (Write XOR eXecute policy) for CUDA kernels.

The protection comes in the form of a dynamic library to be injected with `LD_PRELOAD` when launching the host process using CUDA.

The `Tools/` folder contains some helper scripts for analysis, evaluation and testing, each one is described in its subfolder.

# Compatibility

This tool is compatible with NVIDIA GPUs, from the Ampere (`sm_80`) architecture, when managed by CUDA toolkit version 12.x.x

> _**Note**_: CUDA Toolkit version >= 13 changed the format of compressed FATBINs, as well as the signature of some functions we need to hook.
Adding compatibility requires some Reverse Engineering efforts.

# Platform tailoring

The identification of GPU memory pages holding the kernel's code and the device runtime is based on the order in which context creation will allocate pages.
To identify what is the order on your platform (which depends on the GPU architecture and exact version of the CUDA runtime), you can run the tool in [Tools/findAllocations](./Tools/findAllocations/README.md), and then update the definitions in [cuWX/src/runtime/allocations.h](cuWX/src/runtime/allocations.h), as an example:

```c
#define CUDART_CODE_ALLOCATION_ID 30 // <-- given by the findAllocations tool
#define KERNEL_CODE_ALLOCATION_ID 32 // <-- given by the findAllocations tool
#define CONTEXT_INIT_ALLOCATION_MIN 32 // <-- max between the two
```


### Builing cuWX

Building cuWX is as easy as:

```bash
cd cuWX
make
```

You can set some environment variables to tune the compilation:

- `VERBOSE=1` --> Enable verbose logging
- `PROD=1` --> Disable any logging (errors are still printed)
- `NO_RO_CODE=1` --> cuWX will not set code pages as Read-Only, it will only insert the instrumentation
- `EVALUATION=1` --> Used by the evaluation script, print statistics and overwrite the `time` function to disable *random* test case behaviour
- `LOADTIME_EVAL=1` --> Used by the evaluation script to evaluate the loadtime overhead, measure the time from the start (library constructor) to when the first GPU kernel is invoked, and kill the process.

### Using cuWX

Using cuWX is as easy as:

```bash
LD_PRELOAD=/path/to/cudep.so ./application
```

You can also use cuWX inside cuda-gdb:

```bash
cuda-gdb --init-eval-command="set env LD_PRELOAD /path/to/cudep.so" ./application
```

You can also invoke cuDEP explicitly to patch a binary, in that case it will apply the instrumentation as if the allocation for kernel code was at the address hardcoded in `cuWX/src/cuELF/executables/main.cpp`.

```bash
cd cuWX
make patcher
./patcher <input cuELF> <output cuELF>
```

### Examples

Example kernels are available in the `Examples/` folder.
You can compile them by entering each example's directory and running `make`.

- `code_injection_code` shows that it is possible to write in code pages, and running it with cuWX should result in a crash.
- `code_indection_data` shows that it is possible to execute code in data pages, and running it with cuWX should result in a crash.

For more advanced attacks (i.e. the ones described in the paper), refer to [Attacks](./Attacks/README.md)
