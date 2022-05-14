filter: filter.o bmp_io.o median_filter.o
	gcc -o filter filter.o bmp_io.o median_filter.o

filter.o: filter.c bmp.h
	gcc -g -O0 -c -Wall -Wextra filter.c

median_filter.o: median_filter.c bmp.h
	gcc -g -O0 -c -Wall -Wextra median_filter.c

bmp_io.o: bmp_io.c bmp.h
	gcc -g -O0 -c -Wall -Wextra bmp_io.c
	
clean:
	rm -rf *.o filter

indent:
	indent -kr -nut -ts4 median_filter.c filter.c
