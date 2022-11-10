all: 
	gcc -g -Wall -o serial serial.c ppm.c
	mpicc -g -Wall -o parallel parallel.c ppm.c

clean:
	rm -f serial
	rm -f parallel