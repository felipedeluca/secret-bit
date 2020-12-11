# secret-bit

### Author

Felipe Ranzani de Luca

### About:

This is a small project for my Computer Science classes. The goal is to use steganography techiniques with a file system. With this
small program you are able to mount a bitmap image as a folder and drop a single text file into it. The file will be stored inside the
bitmap image. To retrieve the file, mount again the bitmap image and voilá!.


### Disclaimer:

This program is distributed as is and it may contain bugs.


### How to comnpile & run:

Use Makefile (included) or

>gcc -Wall secret-bit.c `pkg-config fuse3 --cflags --libs` bmp.c -lm -D_FILE_OFFSET_BITS=64 -o secret-bit 
>gcc -Wall secret-bit.c `pkg-config fuse3 --cflags --libs` bmp.c -lm -o secret-bit 


### How to run:
><code>./secret-bit <filename.bmp></code>


### Required libs:

libfuse: https://github.com/libfuse/libfuse

### Troubleshooting:

If you get this error message "libfuse3.so.3: cannot open shared object file: No such file or directory" please try the following:

<code>ln -s /usr/local/lib/x86_64-linux-gnu/libfuse3.so.3.10.0 /lib/x86_64-linux-gnu/libfuse3.so.3</code>


### These are the references I used to make this program:

  #### Lorenzo Fontana
    
  https://github.com/fntlnz/fuse-example

#### Mohammed Q.Hussain
		
https://github.com/MaaSTaaR/SSFS/blob/master/ssfs.c

#### Fuse

https://github.com/libfuse/libfuse/blob/master/example/null.c

#### Alejandro Rodriguez

https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/

#### GeekforGeeks:

https://www.geeksforgeeks.org/modify-bit-given-position/
