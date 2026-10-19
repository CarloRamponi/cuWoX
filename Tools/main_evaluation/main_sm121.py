import argparse
import os
import subprocess
import sys
import tempfile
import re
import time

sys.path.insert(0, '..')
from indirect_branches.main import count_indirect_branches

CUDA_PATH = os.environ.get("CUDA_PATH", "/usr/local/cuda")
# NCU_PATH = os.environ.get("NCU_PATH", os.path.join(CUDA_PATH, "bin/ncu"))
NCU_PATH = "/usr/local/cuda-13.0/bin/ncu"
CUOBJDUMP_PATH = os.path.join(CUDA_PATH, "bin/cuobjdump")

if not os.path.exists(CUOBJDUMP_PATH):
    print(f"cuobjdump not found at {CUOBJDUMP_PATH}")
    sys.exit(1)

if not os.path.exists(NCU_PATH):
    print(f"ncu not found at {NCU_PATH}")
    sys.exit(1)

CUDEP_SRC_PATH = os.path.join(os.path.dirname(__file__), "..", "..", "cuWX")
CUDEP_LOADTIME_OVERHEAD_PATH = os.path.join(os.path.dirname(__file__), "cudep_loadtime.so")
CUDEP_RUNTIME_OVERHEAD_PATH = os.path.join(os.path.dirname(__file__), "cudep_runtime.so")
PATCHER_PATH = os.path.join(os.path.dirname(__file__), "patcher")

CHRT_PATH = os.path.join(os.path.dirname(__file__), "chrt")
BOGUS_LOADTIME_LIBRARY_SRC_PATH = os.path.join(os.path.dirname(__file__), "loadtime_library")
BOGUS_LOADTIME_LIBRARY_PATH = os.path.join(BOGUS_LOADTIME_LIBRARY_SRC_PATH, "bogus.so")
BOGUS_RUNTIME_LIBRARY_SRC_PATH = os.path.join(os.path.dirname(__file__), "runtime_library")
BOGUS_RUNTIME_LIBRARY_PATH = os.path.join(BOGUS_RUNTIME_LIBRARY_SRC_PATH, "bogus.so")

CHRT_AVAILABLE = True
if not os.path.exists(CHRT_PATH):
    CHRT_AVAILABLE = False
    print(f"chrt not found in the current directory, copy it and set the capability with the following command:")
    print(f"cp $(which chrt) ./chrt && sudo setcap cap_sys_nice+ep ./chrt")
    print(f"Without chrt, the loadtime and runtime measurements may be less accurate due to scheduling delays.")
    print(f"Proceeding without chrt, but results may be less accurate.")
    # print(f"Do you want to continue without chrt? [y/N]")
    # choice = input().strip().lower()
    # if choice != "y":
    #     sys.exit(1)

def compile_cudep(force_compile):
    
    if not os.path.exists(CUDEP_LOADTIME_OVERHEAD_PATH) or force_compile:
        print(f"Compiling cuDEP from {CUDEP_SRC_PATH} to {CUDEP_LOADTIME_OVERHEAD_PATH}")
        
        if subprocess.run(["make", "clean", "-C", CUDEP_SRC_PATH]).returncode != 0:
            print(f"Failed to clean cuDEP build directory {CUDEP_SRC_PATH}")
            sys.exit(1)

        if subprocess.run(["make", "-C", CUDEP_SRC_PATH, "EVALUATION=1", "LOADTIME_EVAL=1", "PROD=1"]).returncode != 0:
            print(f"Failed to build cuDEP from {CUDEP_SRC_PATH}")
            sys.exit(1)
        
        # copy file
        if subprocess.run(["cp", os.path.join(CUDEP_SRC_PATH, "cudep.so"), CUDEP_LOADTIME_OVERHEAD_PATH]).returncode != 0:
            print(f"Failed to copy cuDEP to {CUDEP_LOADTIME_OVERHEAD_PATH}")
            sys.exit(1)

    if not os.path.exists(CUDEP_RUNTIME_OVERHEAD_PATH) or force_compile:
        print(f"Compiling cuDEP (no RO) from {CUDEP_SRC_PATH} to {CUDEP_RUNTIME_OVERHEAD_PATH}")
        if subprocess.run(["make", "clean", "-C", CUDEP_SRC_PATH]).returncode != 0:
            print(f"Failed to clean cuDEP build directory {CUDEP_SRC_PATH}")
            sys.exit(1)

        if subprocess.run(["make", "-C", CUDEP_SRC_PATH, "NO_RO_CODE=1", "EVALUATION=1", "PROD=1"]).returncode != 0:
            print(f"Failed to build cuDEP from {CUDEP_SRC_PATH}")
            sys.exit(1)
        
        # copy file
        if subprocess.run(["cp", os.path.join(CUDEP_SRC_PATH, "cudep.so"), CUDEP_RUNTIME_OVERHEAD_PATH]).returncode != 0:
            print(f"Failed to copy cuDEP to {CUDEP_RUNTIME_OVERHEAD_PATH}")
            sys.exit(1)

    if not os.path.exists(PATCHER_PATH) or force_compile:
        if subprocess.run(["make", "clean", "-C", CUDEP_SRC_PATH]).returncode != 0:
            print(f"Failed to clean cuDEP build directory {CUDEP_SRC_PATH}")
            sys.exit(1)

        if subprocess.run(["make", "patcher", "-C", CUDEP_SRC_PATH, "EVALUATION=1", "PROD=1"]).returncode != 0:
            print(f"Failed to build patcher from {CUDEP_SRC_PATH}")
            sys.exit(1)
        
        # copy file
        if subprocess.run(["cp", os.path.join(CUDEP_SRC_PATH, "patcher"), PATCHER_PATH]).returncode != 0:
            print(f"Failed to copy patcher to {PATCHER_PATH}")
            sys.exit(1)

    if not os.path.exists(BOGUS_LOADTIME_LIBRARY_PATH) or force_compile:
        if subprocess.run(["make", "clean", "-C", BOGUS_LOADTIME_LIBRARY_SRC_PATH]).returncode != 0:
            print(f"Failed to clean bogus library build directory {BOGUS_LOADTIME_LIBRARY_SRC_PATH}")
            sys.exit(1)

        if subprocess.run(["make", "-C", BOGUS_LOADTIME_LIBRARY_SRC_PATH]).returncode != 0:
            print(f"Failed to build bogus library from {BOGUS_LOADTIME_LIBRARY_SRC_PATH}")
            sys.exit(1)

    if not os.path.exists(BOGUS_RUNTIME_LIBRARY_PATH) or force_compile:
        if subprocess.run(["make", "clean", "-C", BOGUS_RUNTIME_LIBRARY_SRC_PATH]).returncode != 0:
            print(f"Failed to clean bogus library build directory {BOGUS_RUNTIME_LIBRARY_SRC_PATH}")
            sys.exit(1)

        if subprocess.run(["make", "-C", BOGUS_RUNTIME_LIBRARY_SRC_PATH]).returncode != 0:
            print(f"Failed to build bogus library from {BOGUS_RUNTIME_LIBRARY_SRC_PATH}")
            sys.exit(1)

def measure_memory(test_case):
    print(f"\tMemory overhead...")
    stats = {
        "size": {
            "original": 0,
            "patched": 0,
            "increase": 0
        },
        "instructions": {
            "original": 0,
            "patched": 0,
            "increase": 0
        },
        "instructions_no_padding": {
            "original": 0,
            "patched": 0,
            "increase": 0
        },
        "returns": {
            "total": 0,
            "instrumented": 0
        },
        "cf_transfers": {
            "safe": 0,
            "unsafe": 0
        }
    }

    try:
        with tempfile.TemporaryDirectory("cuDEP_eval") as temp_dir:

            output = subprocess.run([
                CUOBJDUMP_PATH,
                "--extract-elf", "sm_121",
                test_case["path"]
            ], cwd=temp_dir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

            if(output.returncode != 0):
                raise RuntimeError(f"Failed to extract ELF from {test_case['path']}")

            cubins = []
            
            for line in output.stdout.decode().splitlines():
                match = re.match(r"Extracting ELF file\s+\d+: (.+\.sm_121\.cubin)", line)
                if match:
                    cubin_file = match.group(1)
                    # check whether the ELF file contains device code.
                    readelf = subprocess.run([
                        "readelf", "-s", cubin_file
                    ], cwd=temp_dir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                    if readelf.returncode == 0 and "FUNC" in readelf.stdout.decode():
                        cubins.append(cubin_file)
            
            for cubin in cubins:
                patcher = subprocess.run([
                    PATCHER_PATH, cubin, f"{cubin.split('.cubin')[0]}_patched.cubin"
                ], cwd=temp_dir, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

                if patcher.returncode != 0:
                    print(f"Failed to patch {cubin}:")
                    print(patcher.stderr.decode())
                    continue

                output = patcher.stdout.decode()
                # print(output)

                code_size_orig = re.search(r"Original Code Size: (\d+)", output, re.MULTILINE)
                code_size_instr = re.search(r"Instrumented Code Size: (\d+)", output, re.MULTILINE)
                instr_count_orig = re.search(r"Original Instruction Count \(excluding NOP padding\): (\d+)", output, re.MULTILINE)
                instr_count_instr = re.search(r"Instrumented Instruction Count  \(excluding NOP padding\): (\d+)", output, re.MULTILINE)
                ret_count_tot = re.search(r"Return Instruction Count: (\d+)", output, re.MULTILINE)
                ret_count_instr = re.search(r"Instrumented Return Instruction Count: (\d+)", output, re.MULTILINE)

                code_size_orig = int(code_size_orig.group(1), 10)
                code_size_instr = int(code_size_instr.group(1), 10)
                instr_count_orig = int(instr_count_orig.group(1), 10)
                instr_count_instr = int(instr_count_instr.group(1), 10)
                ret_count_tot = int(ret_count_tot.group(1), 10)
                ret_count_instr = int(ret_count_instr.group(1), 10)

                stats["size"]["original"] += code_size_orig
                stats["size"]["patched"] += code_size_instr
                stats["instructions"]["original"] += code_size_orig // 16
                stats["instructions"]["patched"] += code_size_instr // 16
                stats["instructions_no_padding"]["original"] += instr_count_orig
                stats["instructions_no_padding"]["patched"] += instr_count_instr
                stats["returns"]["total"] += ret_count_tot
                stats["returns"]["instrumented"] += ret_count_instr

        stats["size"]["increase"] = (stats["size"]["patched"] / stats["size"]["original"] - 1) if stats["size"]["original"] > 0 else 0
        stats["instructions"]["increase"] = (stats["instructions"]["patched"] / stats["instructions"]["original"] - 1) if stats["instructions"]["original"] > 0 else 0
        stats["instructions_no_padding"]["increase"] = (stats["instructions_no_padding"]["patched"] / stats["instructions_no_padding"]["original"] - 1) if stats["instructions_no_padding"]["original"] > 0 else 0

        indirect_stats = count_indirect_branches(test_case["path"])
        stats["cf_transfers"]["safe"] = indirect_stats["brx"]["safe"] + indirect_stats["jmx"]["safe"] + indirect_stats["call"]["safe"]
        stats["cf_transfers"]["unsafe"] = indirect_stats["brx"]["unsafe"] + indirect_stats["jmx"]["unsafe"] + indirect_stats["call"]["unsafe"]

        return stats
    except Exception as e:
        print(f"Error processing (memory) {test_case['name']}: {e}")
        return {
            "size": {
                "original": "N/A",
                "patched": "N/A",
                "increase": "N/A"
            },
            "instructions": {
                "original": "N/A",
                "patched": "N/A",
                "increase": "N/A"
            },
            "instructions_no_padding": {
                "original": "N/A",
                "patched": "N/A",
                "increase": "N/A"
            },
            "returns": {
                "total": "N/A",
                "instrumented": "N/A"
            },
            "cf_transfers": {
                "safe": "N/A",
                "unsafe": "N/A"
            }
        }


def loadtime(test_case, instrumented=False):
    if instrumented:
        cudep_path = CUDEP_LOADTIME_OVERHEAD_PATH
    else:
        cudep_path = BOGUS_LOADTIME_LIBRARY_PATH

    if CHRT_AVAILABLE:
        command = [
            # "taskset", "-a", "-c", "20-27",
            CHRT_PATH, "-b", "-f", "99",
            "bash", "-c", f"source /home/carlo.ramponi/cuWX/Tools/main_evaluation/bashrc && LD_PRELOAD={cudep_path} {test_case['cmdline']}"
        ]
    else:
        command = [
            "bash", "-c", f"source /home/carlo.ramponi/cuWX/Tools/main_evaluation/bashrc && LD_PRELOAD={cudep_path} {test_case['cmdline']}"
        ]

    exec = subprocess.run(command, capture_output=True, text=True, cwd=test_case["cwd"])
    # we don't care if the program crashed, as long as it has printed the loadtime time
    # if exec.returncode != 0:
    #     raise RuntimeError(f"Program crashed with return code {exec.returncode}")
    
    loadtime_output = exec.stdout
    loadtime_match = re.search(r"\[cuDEP\]: Loadtime evaluation time: ([\d\.]+) milliseconds", loadtime_output, re.MULTILINE)
    if loadtime_match:
        return float(loadtime_match.group(1))

    print("cwd: " + test_case["cwd"])
    print("Command: " + " ".join(command))
    print(loadtime_output)
    print(exec.stderr)
    raise RuntimeError("Failed to parse load time")

def runtime(test_case, instrumented=False):
    with tempfile.NamedTemporaryFile(delete=False, prefix="runtime_", suffix=".csv", dir="./tmp") as temp_file:
        command = [
            "sudo", NCU_PATH, "--null-stdin", "--csv",
            "-c", "50", "--replay-mode", "kernel",
            "--metrics", "smsp__thread_inst_executed,gpu__time_duration",
            "--log-file", temp_file.name,
            "--preload", CUDEP_RUNTIME_OVERHEAD_PATH if instrumented else BOGUS_RUNTIME_LIBRARY_PATH,
            "bash", "-c",
        ]

        cmdline = test_case["cmdline"].split(" ")

        bash_cmd = f"source /home/carlo.ramponi/cuWX/Tools/main_evaluation/bashrc && "
        # bash_cmd = ""
    
        temp = None
        for arg in cmdline:
            if arg.startswith("\""):
                temp = arg[1:]
            elif arg.endswith("\"") and temp is not None:
                bash_cmd += temp + " " + arg[:-1] + " "
                temp = None
            elif temp is not None:
                temp += " " + arg
            else:
                bash_cmd += arg + " "
        
        command.append(f"{bash_cmd.strip()}")


        exec = subprocess.run(command, capture_output=True, text=True, cwd=test_case["cwd"])

        if exec.returncode != 0:
            print("cwd: " + test_case["cwd"])
            print("Command: " + " ".join(command))
            print(exec.stdout)
            print(exec.stderr)
            # raise RuntimeError(exec.stdout)

    with open(temp_file.name, 'r') as f:
        runtime_output = f.read()

    # print(runtime_output)
    
    instructions_matches = re.findall(r"\"smsp__thread_inst_executed\.sum\",\"inst\",\"([\d\.,]+)\"", runtime_output, re.MULTILINE)
    time_matches = re.findall(r"\"gpu__time_duration\.sum\",\"ns\",\"([\d\.,]+)\"", runtime_output, re.MULTILINE)

    instructions = []
    times = []

    # print(f"Instructions matches: {instructions_matches}")
    # print(f"Time matches: {time_matches}")

    for match in instructions_matches:
        instructions.append(int(match.replace(",", ".").replace(".", "")))

    for match in time_matches:
        times.append(int(match.replace(",", ".").replace(".", "")))

    # print(f"Instructions: {instructions}")
    # print(f"Times: {times}")

    if not instructions or not times:
        raise RuntimeError("Failed to parse runtime")
    
    instructions = sum(instructions)
    times = sum(times)

    return instructions, times

def measure_runtime(test_case, runtime_iter=5):
    print("\tRuntime overhead...")
    stats =  {
        "instructions": {
            "original": 0,
            "patched": 0,
            "increase": 0
        },
        "time": {
            "original": 0,
            "patched": 0,
            "increase": 0
        }
    }

    try:
        cycles_orig, time_orig = [], []
        cycles_patched, time_patched = [], []

        for i in range(runtime_iter):
            print(f"\t\tIteration {i+1}/{runtime_iter}...", end="\r")
            c, t = runtime(test_case, instrumented=True)
            cycles_patched.append(c)
            time_patched.append(t)

            c, t = runtime(test_case, instrumented=False)
            cycles_orig.append(c)
            time_orig.append(t)
        print()

        stats["instructions"]["patched"] = sum(cycles_patched) / len(cycles_patched) if cycles_patched else 0
        stats["time"]["patched"] = sum(time_patched) / len(time_patched) if time_patched else 0

        stats["instructions"]["original"] = sum(cycles_orig) / len(cycles_orig) if cycles_orig else 0
        stats["time"]["original"] = sum(time_orig) / len(time_orig) if time_orig else 0

        stats["instructions"]["increase"] = (stats["instructions"]["patched"] / stats["instructions"]["original"] - 1) if stats["instructions"]["original"] > 0 else 0
        stats["time"]["increase"] = (stats["time"]["patched"] / stats["time"]["original"] - 1) if stats["time"]["original"] > 0 else 0
        
        return stats
    except Exception as e:
        print(f"Error processing (runtime) {test_case['name']}: {e}")
        return {
            "instructions": {
                "original": "N/A",
                "patched": "N/A",
                "increase": "N/A"
            },
            "time": {
                "original": "N/A",
                "patched": "N/A",
                "increase": "N/A"
            }
        }

def measure_loadtime(test_case, loadtime_iter=5):
    print("\tLoad-time overhead...")
    stats = {
        "time": {
            "original": 0,
            "patched": 0,
            "increase": 0
        }
    }

    try:
        original = []
        patched = []
        for i in range(loadtime_iter):
            print(f"\t\tIteration {i+1}/{loadtime_iter}...", end="\r")
            patched.append(loadtime(test_case, instrumented=True))
            original.append(loadtime(test_case, instrumented=False))
        print()
        stats["time"]["original"] = sum(original) / len(original)
        stats["time"]["patched"] = sum(patched) / len(patched)
        stats["time"]["increase"] = (stats["time"]["patched"] / stats["time"]["original"] - 1) if stats["time"]["original"] > 0 else 0

        return stats
    except Exception as e:
        print(f"Error processing (loadtime) {test_case['name']}: {e}")
        return {
            "time": {
                "original": "N/A",
                "patched": "N/A",
                "increase": "N/A"
            }
        }


def process_test_case(test_case, runtime_iter=5, loadtime_iter=5, only_memory=False):
    print(f"Processing test case: {test_case['name']}")
    stats = {
        "memory": measure_memory(test_case),
        "runtime": None if only_memory else measure_runtime(test_case, runtime_iter=runtime_iter),
        "loadtime": None if only_memory else measure_loadtime(test_case, loadtime_iter=loadtime_iter)
    }
    return stats

def main(test_cases_file, output_file, force_compile=False, runtime_iter=5, loadtime_iter=5, only_memory=False):
    compile_cudep(force_compile)

    with open(test_cases_file, 'r') as f:
        test_cases = f.readlines()

    test_cases = [t.strip().split(";") for t in test_cases]
    test_cases = [{"name": t[0], "path": t[1], "cwd": t[2], "cmdline": t[3]} if len(t) == 4 else None for t in test_cases]

    with open(output_file, 'a') as f:
        # Write date and time
        f.write(f"\nEvaluation run on: {time.asctime()}. Runtime iterations: {runtime_iter}. Load-time iterations: {loadtime_iter}\n")
        f.flush()
        for test_case in test_cases:

            if test_case is None:
                f.write("\n")
                continue

            result = process_test_case(test_case, runtime_iter=runtime_iter, loadtime_iter=loadtime_iter, only_memory=only_memory)

            f.write("\t".join([
                test_case["name"],
                str(result["memory"]["size"]["original"]),
                str(result["memory"]["size"]["patched"]),
                str(result["memory"]["size"]["increase"]),
                str(result["memory"]["instructions"]["original"]),
                str(result["memory"]["instructions"]["patched"]),
                str(result["memory"]["instructions"]["increase"]),
                str(result["memory"]["instructions_no_padding"]["original"]),
                str(result["memory"]["instructions_no_padding"]["patched"]),
                str(result["memory"]["instructions_no_padding"]["increase"]),
                str(result["memory"]["returns"]["total"]),
                str(result["memory"]["returns"]["instrumented"]),
                str(result["memory"]["cf_transfers"]["safe"]),
                str(result["memory"]["cf_transfers"]["unsafe"])
            ] + ([] if only_memory else [
                str(result["runtime"]["instructions"]["original"]),
                str(result["runtime"]["instructions"]["patched"]),
                str(result["runtime"]["instructions"]["increase"]),
                str(result["runtime"]["time"]["original"]),
                str(result["runtime"]["time"]["patched"]),
                str(result["runtime"]["time"]["increase"]),
                str(result["loadtime"]["time"]["original"]),
                str(result["loadtime"]["time"]["patched"]),
                str(result["loadtime"]["time"]["increase"]),
            ])))
            f.write("\n")
            f.flush()

if __name__ == "__main__":
    # Parse arguments
    parser = argparse.ArgumentParser(description="Compute runtime, loadtime and memory overhead")
    parser.add_argument("-f", "--file", help="Path to the csv file containing the list of testcases", required=True)
    parser.add_argument("-o", "--output", help="Path to the output file", required=True)
    parser.add_argument("--force-compile", action="store_true", help="Force recompilation of cuDEP", default=False)
    parser.add_argument("--runtime-iter", type=int, default=5, help="Number of iterations for runtime measurement")
    parser.add_argument("--loadtime-iter", type=int, default=5, help="Number of iterations for loadtime measurement")
    parser.add_argument("--only-memory", action="store_true", help="Only measure memory overhead", default=False)

    args = parser.parse_args()

    main(args.file, args.output, args.force_compile, args.runtime_iter, args.loadtime_iter, args.only_memory)