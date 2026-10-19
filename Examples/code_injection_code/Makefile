all:
	nvcc -G app.cu -o app --cudart shared -arch=all --no-compress #-Xfatbin --compress-all
	#nvcc -G -arch=sm_80 app.cu -o app --cudart shared -Xfatbin --compress-all

clean:
	rm -f app
