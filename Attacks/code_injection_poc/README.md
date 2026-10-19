# Shellcode Injection into Data Memory

This PoC allows to exploit the same program with and without ASLR.

### Attack phases
The attack is divided in two phases:
1. **Address leaking** needed to obtain the addresses to mount the attack. We need two addresses: **res** and **vec** variables. 
2. **Shellcode injection**: once obtained the addresses, the former is used to overwrite a known memory location with some data and the latter is where our shellcode is resting and used as new return address (RA).

> [!NOTE] 
> **Assumption 1**: Given that ASLR & STACK CANARIES cause address changes only across process instances, we assume that the program is executed only once, allowing the user to interact with it an arbitrary number of times within the same process.
> 
> **Assumption 2**: We assume that the attacker knows that `res` is at an offset of `0x1000` from `vec`. 

### Exploitation

The expected behavior of the program is to print the sum of a series of numbers provided as input. However, when the number of inputs (excluding the terminating -1) exceeds 256, the program returns the correct sum plus an **unknown value** — leaked stack information. Since we can easily calculate the sum of our input, the unknown value can be retrieve just by subtracting the expected sum. This process can be optimized by providing a series of 0s as input - see [PoC](./exploit.py).

Given that it is possible to extract the stack address at position `input_len+1` we can increment the payload length one by one. By doing so it is possible to recover the whole stack content.

Some **adjustments** to the payload may be required to avoid "Illegal memory access" errors as we progress through the stack. These are typically caused when the payload overwrites a previously discovered valid address with a 0. This issue can be solved by replacing the 0 with the previously leaked address. 

> [!NOTE]
> Using this method enables us to extract and use the **stack canaries** that may be present in the program instance (more details later).

The sum printed by the program can be: the correct address (first address case), or a "random" address (sum of addresses or overflowed number - not a problem). To retrieve the correct one we just need to get the current number `i` and subtract the previous sum `i-1`. 

Below two example without aslr
```bash
['0xdf3db600', '0x61bb6c7c', '0x7fff', '0xd8201000', '0x0', '0x0', '0x0', '0xdf26e580', '0x7fff']
```
and with aslr.
```bash
['0x32abe600', '0x3b31333c', '0x7620', '0x84201000', '0x0', '0x0', '0x0', '0x8b26e580', '0x7620']
```

> [!NOTE]
> For clarity purposes, I will be using addresses without ASLR enabled from here. No ASLR addresses might change depending on drivers or whether you run the program with `cuda-gdb` or not. For example `['0x32abe600', '0x3b31333c', '0x7fff', '0xd8601000', '0x0', '0x0', '0x0', '0xdd26c480', '0x7fff']`.

As you can see, the structure does not change much, only some bits are randomized due to ASLR. Since NVIDIA Ampere uses a 32-bit memory interface, we see the values split across multiple entries in the array. Usually, the two byte part is the **prefix** (e.g. `0x7fff`) which is common across areas of memory (global/shared/local). The addresses that we are able to retrieve with this PoC are:
- **stack canaries**: higher `0xdf3db600` and lower `0x61bb6c7c` parts
- **res variable**: `0x7fffd8201000` 
- **return address**: `0x7fffdf26e580`

The SASS instructions of the shellcode are shown below. The addresses used for **res** and **vec** (`vec = res_addr - 0x1000`), which serve as the base address for the shellcode, change due to ASLR.

```asm
.headerflags    @"EF_CUDA_64BIT_ADDRESS EF_CUDA_SM80 EF_CUDA_VIRTUAL_SM(EF_CUDA_SM80)"
/*7fffd8200000*/   MOV R4, 0xd8201000 ;
/*7fffd8200010*/   MOV R5, 0x7fff ;
/*7fffd8200020*/   MOV R0, 0xdeadbeef ;
/*7fffd8200030*/   ST.E [R4.64], R0 ;
/*7fffd8200040*/   EXIT ;
```

To load the shellcode into memory, we first need to convert it into a series of numbers (as required by our program) and then concatenate an arbitrary number of return addresses (RAs) until we find the correct offset. In this case, we have also enabled a stack canary, which must be placed at a specific offset to avoid triggering an error. The canary and RA are located at offsets 128 and 131, respectively, after the shellcode.

The expected output of a successful exploit is something like this:
```bash
=== LEAKED ADDRESSES ===
['0xdf3db600', '0x61bb6c7c', '0x7fff', '0xd8201000', '0x0', '0x0', '0x0', '0xdf26e580', '0x7fff']
=== RESULT AFTER FN INJECT ===
b'3735928559 - 0x00000000deadbeef\n'
```
