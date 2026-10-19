all: app library

INCLUDE_PATHS := $(CUDA_PATH)/include
INCLUDE_PATHS += ../../cuWX/nvidia
INCLUDE_PATHS += ../../cuWX/src/utils
INCLUDE_PATHS += .

library: allocations.cpp ioctl_wrapper.cpp
	g++ -shared $(INCLUDE_PATHS:%=-I%) -fPIC -o liballocations.so allocations.cpp ioctl_wrapper.cpp -ldl

app: app.cu library
	nvcc -G $(INCLUDE_PATHS:%=-I%) -L. -lallocations app.cu -o app --cudart shared -arch=all -Xfatbin --compress-all

clean:
	rm -f app liballocations.so
