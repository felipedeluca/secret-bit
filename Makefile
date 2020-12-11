.PHONY: run clean

mr: bmpfs.c secret-bit.c bmpfs.h 
	gcc -Wall secret-bit.c `pkg-config fuse3 --cflags --libs` bmpfs.c -lm -D_FILE_OFFSET_BITS=64 -o secret-bit 
run:
	./mr

clean:
	@rm -f mr