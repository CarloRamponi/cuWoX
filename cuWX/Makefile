rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

INCLUDE_PATHS := $(CUDA_PATH)/include
INCLUDE_PATHS += ./nvidia
INCLUDE_PATHS += ./src/cuELF
INCLUDE_PATHS += ./src/hooks
INCLUDE_PATHS += ./src/runtime
INCLUDE_PATHS += ./src/utils
INCLUDE_PATHS += ./src/

LINK_PATHS := $(CUDA_PATH)/lib64
LINK_PATHS += $(CUDA_PATH)/lib

# nvptxcompiler_static does not link properly without these flags
LINK_FLAGS := -Wl,-u,nvPTXCompilerCreate,-u,nvPTXCompilerDestroy,-u,nvPTXCompilerGetCompiledProgram,-u,nvPTXCompilerGetProgramLog,-u,nvPTXCompilerCompileProgram

LIBRARIES += dl nvptxcompiler_static

SOURCE_FILES := $(call rwildcard,src,*.c)
SOURCE_FILES += $(call rwildcard,src,*.cpp)

# remove main.cpp from the list of source files
filter = $(filter-out src/cuELF/executables/main.cpp, $(SOURCE_FILES))
SOURCE_FILES := $(filter)

# remove dump_nvinfo.cpp from the list of source files
filter = $(filter-out src/cuELF/executables/dump_nvinfo.cpp, $(SOURCE_FILES))
SOURCE_FILES := $(filter)

TOOL_SRC_FILES := $(wildcard src/cuELF/*.cpp)

CFLAGS := -std=c++20

WARNING_CFLAGS := format=0
WARNING_CFLAGS += no-packed-bitfield-compat
WARNING_CFLAGS += no-reorder

PREPROCESSOR_DEFINES := _GNU_SOURCE

ifdef VERBOSE
	PREPROCESSOR_DEFINES += VERBOSE
endif

ifdef PROD
	PREPROCESSOR_DEFINES += PROD
endif

ifndef NO_RO_CODE
	PREPROCESSOR_DEFINES += SET_RO_CODE
endif

ifdef EVALUATION
	PREPROCESSOR_DEFINES += EVALUATION
endif

ifdef LOADTIME_EVAL
	PREPROCESSOR_DEFINES += LOADTIME_EVAL
endif

FINAL_FLAGS = $(CFLAGS) $(LINK_FLAGS) $(WARNING_CFLAGS:%=-W%) $(INCLUDE_PATHS:%=-I%) $(LIBRARIES:%=-l%) $(PREPROCESSOR_DEFINES:%=-D%) $(LINK_PATHS:%=-L%) $(LINK_EXPLICITLY:%=-l:%)

all: cudep.so

cudep.so: $(SOURCE_FILES) $(INCLUDE_PATHS)
	g++ -shared -fPIC $(FINAL_FLAGS) -o $@ $(STATIC_LIBRARIES) $(SOURCE_FILES)

patcher: $(TOOL_SRC_FILES) $(INCLUDE_PATHS) src/cuELF/executables/main.cpp
	g++ -o $@ $(FINAL_FLAGS) $(TOOL_SRC_FILES) $(STATIC_LIBRARIES) src/cuELF/executables/main.cpp

dump_nvinfo: $(TOOL_SRC_FILES) $(INCLUDE_PATHS) src/cuELF/executables/dump_nvinfo.cpp
	g++ -o $@ $(FINAL_FLAGS) $(TOOL_SRC_FILES) $(STATIC_LIBRARIES) src/cuELF/executables/dump_nvinfo.cpp

clean:
	rm -f cudep.so dump_nvinfo patcher
