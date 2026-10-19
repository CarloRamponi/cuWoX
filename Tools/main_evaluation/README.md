# cuDEP Evaluation

```bash
usage: main.py [-h] -f FILE -o OUTPUT [--force-compile] [--runtime-iter RUNTIME_ITER] [--loadtime-iter LOADTIME_ITER]

Compute runtime, loadtime and memory overhead

options:
  -h, --help            show this help message and exit
  -f FILE, --file FILE  Path to the csv file containing the list of testcases
  -o OUTPUT, --output OUTPUT
                        Path to the output file
  --force-compile       Force recompilation of cuDEP
  --runtime-iter RUNTIME_ITER
                        Number of iterations for runtime measurement
  --loadtime-iter LOADTIME_ITER
                        Number of iterations for loadtime measurement
```

### Input File format

The input file is a CSV file containing the infomation needed to start every testcase.
The structure is the following:

```csv
name;bin_path;workdir;cmdline
bfs;/home/xxxx/gpu-rodinia/cuda/bfs/bfs;/home/xxxx/gpu-rodinia/cuda/bfs;./bfs arg1 arg2 arg3
```

The output file will contain a tab separated csv file (so that you copy-paste it into google sheets) with this values, in order:

- name of the testcase
- size of the code sections (bytes): original, patched and increase
- number of SASS instructions: original, patched and increase
- number of SASS instructions excluding the NOP padding: original, patched and increase
- number of return instructions: total and instrumented
- number of control flow transfers (non RET): safe and unsafe
- number of executed SASS instructions: original, patched and increase
- GPU execution time (ns): original, patched and increase
- loadtime overhead (ms) (from program launch to first kernel launch): original, patched and increase

Example:

```
llama3	32896	33152	0.0077821011673151474	2056	2072	0.0077821011673151474	2000	2015	0.007500000000000062	8	3	0	0	92851740.0	92851794.0	5.815723000957007e-07	201734.4	201766.4	0.00015862440912406406	129.2469491	131.5769841	0.018027775635904852
```
