all:
# 	nvcc -G app.cu -o app --cudart shared --gpu-architecture=compute_80 --gpu-code=compute_80 -Xfatbin --compress-all
	nvcc -G -arch=all app.cu -o app --cudart shared --no-compress #-Xfatbin --compress-all

clean:
	rm -f app
