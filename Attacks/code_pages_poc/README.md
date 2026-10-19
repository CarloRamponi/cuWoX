# Direct Overwrite of Code Memory

## Application task

Given **N** and **offset** provided by the user, the applications gets as input an array of integer of size N, and shifts it by the provided offset applying a series of operations (sum and multiplication) on the provided input. The result is an array containing the modified user input shifted by offset. 

## Assumptions

For simplicity the user is provided with two addresses:
- **array** that the attacker wants to overwrite
- one **function** address

In a real world scenario we assume the attacker can successfully leak the required memory addresses. Currently, emulating this scenario is challenging given that GPUs heavily rely on registers, and we lack sufficient understanding of how addresses are managed in larger applications. However, our main goal is to show the possibility of overwriting code pages. 

## Use exploit

The exploit is available in two versions, that is two different shellcode can be generated with `gen_payload` (`--help` for more info):
1. a shellcode that overwrite the all the values **N** with `0xdeadbeef` (default)
2. a shellcode that overwrites provided address with `0xdeadbeef` value

First, we need to obtain the addresses from the application. At this purpose we just provide **N**, **input** and **offset** and together with our shifted array we will have `d_destination` and `mul_imm` addresses. 

Accordingly to which shellcode we want to use, we need to calculate the address slightly differently:
1. `((mul_imm+64)-d_destination)/4`
2. `(mul_imm-d_destination)/4`

>[!NOTE]
> The offset has to be **divided by 4** because our input is an array of **int** which addresses the memory with `(offset * sizeof(int)). 
> For the first shellcode we also need to add **64** to `mul_imm` because the instruction we want to overwrite is the fifth, and since SASS instruction are 128bit encoded we consider a 64B offset starting from the function base address.

Now we can:
1. generate the payload with `python3 gen_payload.py` or `python3 gen_payload --addr <d_destionation>`
2. input a new **N** (the crafted payload should be 4 or 20)
3. input the **payload**
4. input the obtained **offset**

> [!NOTE]
> Regardless of we launch the program with or without aslr the **offset will not change** since only the base address is changing.

## Explanation

The shellcode used for the single address overwrite is the same seen in the **stack code injection poc**, and I will not go into the details. Instead the second shellcode is shorter however shows the potential of overwriting a lot of values inside an allocated array (e.g. overwriting the weights of an LLM). 

We dont have any knowledge about how SASS handles return values and which registers may use, however by looking at the SASS code of our program I noticed that in this case the `R4` register appeard to be used to store the return value of the `mul_imm` function which is then stored in global memory using `STG.E [R2.64], R4`. Knowing this, I tried to overwrite the `MOV R4, R4` instruction inside `mul_imm` with `MOV R4, 0xdeadbeef` and the result was that given a length **N** as size, all the number provided will be overwritten with `0xdeadbeef`. 

```
/*0000*/    MOV R4, R4 ;               
/*0010*/    MOV R4, R4 ;               
/*0020*/    SHF.L.U32 R4, R4, 0x1, RZ ;
/*0030*/    MOV R4, R4 ;               
/*0040*/    MOV R4, R4 ;               
/*0050*/    RET.ABS.NODEC R20 0x0 ;    
/*0060*/    BRA 0x60;                  
```

```
/*0000*/    MOV R4, R4 ;               
/*0010*/    MOV R4, R4 ;               
/*0020*/    SHF.L.U32 R4, R4, 0x1, RZ ;
/*0030*/    MOV R4, R4 ;               
/*0040*/    MOV R4, 0xdeadbeef ;               
/*0050*/    RET.ABS.NODEC R20 0x0 ;    
/*0060*/    BRA 0x60;                  
```
