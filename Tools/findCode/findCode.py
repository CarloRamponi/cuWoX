import argparse
import subprocess
import tempfile
from multiprocessing.pool import ThreadPool
from alive_progress import alive_bar

def disassemble_instruction(instruction, base_address):
    """Disassemble a single instruction using nvdisasm."""
    with tempfile.NamedTemporaryFile(delete=True) as tmp:
        tmp.write(instruction)
        tmp.flush()
        cmd = f"nvdisasm --binary SM80 --base-address 0x{base_address:x} {tmp.name}"
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        if result.returncode != 0:
            return f"\t/*{base_address:04x}*/\t\t{instruction.hex()} // Invalid instruction"
        else:
            return result.stdout.strip().splitlines()[-1]  # Get the last line of output

def main(binary_file, output_file):
    try:
        with open(binary_file, 'rb') as f:
            length = f.seek(0, 2)  # Get the length of the file
            f.seek(0)  # Reset to the beginning of the file
            with open(output_file, 'w') as out_f:
                with alive_bar(length // 16, title="Disassembling instructions") as bar:
                    with ThreadPool() as pool:
                        for result in pool.imap(lambda x: disassemble_instruction(f.read(16), x), range(0, length, 16)):
                            if out_f:
                                out_f.write(result + "\n")
                            else:
                                print(result)
                            bar()

    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Disassemble a binary file, ignoring invalid instructions.")
    parser.add_argument("binary_file", help="Path to the binary file to disassemble")
    parser.add_argument("--output", "-o", help="Output file to save the disassembled instructions", required=True)

    args = parser.parse_args()

    main(args.binary_file, args.output)
