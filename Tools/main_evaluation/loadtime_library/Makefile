INCLUDE_PATHS := $(CUDA_PATH)/include

SOURCE_FILES := src.c

CFLAGS := -std=c++20

PREPROCESSOR_DEFINES := _GNU_SOURCE

all: bogus.so

bogus.so: $(SOURCE_FILES) $(INCLUDE_PATHS)
	g++ -shared -fPIC $(INCLUDE_PATHS:%=-I%) $(PREPROCESSOR_DEFINES:%=-D%) $(CFLAGS) -o $@ $(SOURCE_FILES)

clean:
	rm -f bogus.so