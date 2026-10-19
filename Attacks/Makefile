# Allow specifying a folder, default to current directory
ifndef DIR
$(info [INFO] DIR not set — using current directory)
DIR = .
else
$(info [INFO] Found DIR - using $(DIR))
endif

# Allow specifying a single file for single builds
ifndef SRC
$(info [INFO] SRC not set — using default: $(DIR)/src.cu)
SRC = $(DIR)/src.cu
else
$(info [INFO] Found SRC - using $(SRC))
endif

# Find all .cu/.cpp source files in the specified directory
CU_SOURCES  := $(wildcard $(DIR)/*.cu)
CPP_SOURCES := $(wildcard $(DIR)/*.cpp)
objects     := $(CU_SOURCES:.cu=.o) $(CPP_SOURCES:.cpp=.o)

NVCCFLAGS   = --cudart shared --no-compress --device-stack-protector true -arch=sm_80
PTXASFLAGS  = -Xptxas -O0

# Ensure output directory exists
$(shell mkdir -p $(DIR))

all: clean $(objects)
	nvcc $(PTXASFLAGS) $(NVCCFLAGS) $(objects) -o $(DIR)/app.out

debug: NVCCFLAGS += -G -g
debug: clean $(objects)
	nvcc $(NVCCFLAGS) $(PTXASFLAGS) $(objects) -o $(DIR)/app.out

# Compile .cpp or .cu sources from the target directory
$(DIR)/%.o: $(DIR)/%.cpp
	nvcc -x cu $(PTXASFLAGS) $(NVCCFLAGS) -I$(DIR) -dc $< -o $@

$(DIR)/%.o: $(DIR)/%.cu
	nvcc $(PTXASFLAGS) $(NVCCFLAGS) -I$(DIR) -dc $< -o $@

# Single file compilation
single:
	nvcc $(PTXASFLAGS) $(NVCCFLAGS) $(SRC) -o $(DIR)/app.out

single-debug:
	nvcc $(PTXASFLAGS) $(NVCCFLAGS) -G -g $(SRC) -o $(DIR)/app.out

clean:
	rm -f $(DIR)/app.out $(DIR)/app.cubin $(DIR)/app.cuasm $(DIR)/ptx-elf.txt $(DIR)/sass-elf.txt $(DIR)/disas.txt
	rm -f $(DIR)/*.o $(DIR)/app
	rm -rf $(DIR)/keep
