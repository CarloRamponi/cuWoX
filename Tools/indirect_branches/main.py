import argparse
import subprocess
import re
import sys
import os

instr_regex = re.compile(r"^/\*[\da-f]+\*/\s+(.+;)\s+/\* 0x[\da-f]+ \*/$")
brx_regex = re.compile(r"^BRX.* (R\d+).*$")
jmx_regex = re.compile(r"^JMX.* (R\d+).*$")
call_regex = re.compile(r"^CALL.* (R\d+).*$")
ret_regex = re.compile(r"^RET.* (R\d+).*$")

def is_safe(register, index, function, function_name=""):
    for i in range(index - 1, 0, -1):
        instr = function[i]
        if re.search(rf"^[^ ]+ {register}[, ].*$", instr):
            if instr.startswith("LDC"):
                # Loading from constant memory: safe
                return True
            elif instr.startswith("LD"):
                # Loading from memory: unsafe
                return False
            elif re.match(rf"^MOV.* {register}, 0x[\da-f]+ ;.*$", instr):
                # Moving an immediate value to the register: safe
                return True
            elif instr.startswith("RET"):
                # Another return instruction before: continue searching
                continue
            else:
                # Unknown instruction pattern: unsafe
                # print(f"Unsafe use of {register} found in instruction: {instr}", file=sys.stderr)
                return False
    # register not found
    print(f"Register not found in previous instructions, assuming safe.", file=sys.stderr)
    # print(f"Function: {function_name}, Register: {register}, Index: {index}, Instr: {function[index]}", file=sys.stderr)
    # return True
    return False

def extract_sass_functions(stream):
    functions = {}
    current_function = None

    for line in stream:
        line = line.decode().strip()

        if line.startswith("Function : "):
            current_function = line.split("Function : ")[1].strip()
            functions[current_function] = []
        elif current_function:
            if line.startswith(".........."):
                # This line indicates the end of the function's SASS code
                yield current_function, functions[current_function]
                current_function = None
                continue
            elif line.startswith("."):
                # This line is a directive or comment, skip it
                continue

            instr = instr_regex.match(line)
            if instr:
                functions[current_function].append(instr.group(1))

def count_indirect_branches(input_file):
    cmd = ["cuobjdump", "-sass", "-arch", "sm_80", input_file]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

    stats = {
        "total_functions": 0,
        "total_instructions": 0,
        "brx": {
            "safe": 0,
            "unsafe": 0
        },
        "jmx": {
            "safe": 0,
            "unsafe": 0
        },
        "call": {
            "safe": 0,
            "unsafe": 0
        },
        "ret": {
            "safe": 0,
            "unsafe": 0
        }
    }

    abs_ret = 0
    rel_ret = 0

    for func_name, func_body in extract_sass_functions(proc.stdout):
        stats["total_functions"] += 1
        stats["total_instructions"] += len(func_body)

        for i in range(len(func_body)):
            instr = func_body[i]

            match = brx_regex.match(instr)
            if match:
                # This is a brx instruction
                safe = is_safe(match.group(1), i, func_body, func_name)
                stats["brx"]["safe"] += 1 if safe else 0
                stats["brx"]["unsafe"] += 1 if not safe else 0
                continue

            match = jmx_regex.match(instr)
            if match:
                # This is a JMX instruction
                safe = is_safe(match.group(1), i, func_body, func_name)
                stats["jmx"]["safe"] += 1 if safe else 0
                stats["jmx"]["unsafe"] += 1 if not safe else 0
                continue

            match = call_regex.match(instr)
            if match:
                # This is a CALL instruction
                safe = is_safe(match.group(1), i, func_body, func_name)
                stats["call"]["safe"] += 1 if safe else 0
                stats["call"]["unsafe"] += 1 if not safe else 0
                continue

            match = ret_regex.match(instr)
            if match:
                # This is a RET instruction
                safe = is_safe(match.group(1), i, func_body, func_name)
                if not safe:
                    if "ABS" in instr:
                        abs_ret += 1
                        print(f"Unsafe RET found in function {func_name} at instruction index {i}: {instr}", file=sys.stderr)
                    else:
                        rel_ret += 1
                stats["ret"]["safe"] += 1 if safe else 0
                stats["ret"]["unsafe"] += 1 if not safe else 0
                continue

    proc.stdout.close()
    proc.wait()

    print(f"Total unsafe RET instructions: {stats['ret']['unsafe']} (ABS: {abs_ret}, REL: {rel_ret})", file=sys.stderr)

    return stats

def main(input_file):
    stats = count_indirect_branches(input_file)

    libname = os.path.basename(input_file).replace("_static.a", "").replace(".so", "")

    print(f"{libname}\t{stats['total_functions']}\t{stats['total_instructions']}\t{stats['brx']['safe']}\t{stats['brx']['unsafe']}\t{stats['jmx']['safe']}\t{stats['jmx']['unsafe']}\t{stats['call']['safe']}\t{stats['call']['unsafe']}\t{stats['ret']['safe']}\t{stats['ret']['unsafe']}")

    # print stats
    # for key, value in stats.items():
    #     if key == "total_functions":
    #         print(f"Total functions: {value}")
    #     elif key == "total_instructions":
    #         print(f"Total instructions: {value}")
    #     else:
    #         print(f"  {key}:")
    #         print(f"    Safe: {value['safe']}")
    #         print(f"    Unsafe: {value['unsafe']}")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Analyze control-flow transfers.")
    parser.add_argument("input_file", help="Path to the input file, must be something readable by cuobjdump (CUBIN, fatbin)")
    args = parser.parse_args()
    main(args.input_file)