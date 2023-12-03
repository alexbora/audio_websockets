

OPT = -Ofast -mavx2 -march=native -mtune=native -fopenmp -std=c99 -fomit-frame-pointer -funroll-loops -ffast-math -fno-strict-aliasing -funsafe-math-optimizations -fno-math-errno -fno-trapping-math -fassociative-math -freciprocal-math -fno-signed-zeros -funsafe-loop-optimizations -fno-stack-protector

all: 
	gcc-13 $(OPT) server_mac.c -o server_mac  

debug: 
	gcc -g -O2 -Wall server_mac.c -o server_mac -DDEBUG
clean: 
	rm server_mac
