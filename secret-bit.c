/*
	Author: Felipe Ranzani de Luca

	About:
		This is a small project for my Computer Science classes. The goal is to use steganography techiniques with a file system. With this
		small program you are able to mount a bitmap image as a folder and drop a single text file into it. The file will be stored inside the
		bitmap image. To retrieve the file, mount again the bitmap image and voilá!.


	Disclaimer:
		This program is distributed as is and it may contain bugs.


    How to comnpile & run:
        gcc -Wall secret-bit.c `pkg-config fuse3 --cflags --libs` bmp.c -lm -D_FILE_OFFSET_BITS=64 -o secret-bit 
        gcc -Wall secret-bit.c `pkg-config fuse3 --cflags --libs` bmp.c -lm -o secret-bit 

	How to run:
		./secret-bit -s -f -o auto_unmount <filename.ext>

	Troubleshooting:
		"libfuse3.so.3: cannot open shared object file: No such file or directory":
			ln -s /usr/local/lib/x86_64-linux-gnu/libfuse3.so.3.10.0 /lib/x86_64-linux-gnu/libfuse3.so.3

    These are the references I used to make this programs:
        Lorenzo Fontana
        https://github.com/fntlnz/fuse-example

		Mohammed Q.Hussain
		https://github.com/MaaSTaaR/SSFS/blob/master/ssfs.c

		Fuse
		https://github.com/libfuse/libfuse/blob/master/example/null.c

        Alejandro Rodriguez
        https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/

        GeekforGeeks:
        https://www.geeksforgeeks.org/modify-bit-given-position/
*/
#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <limits.h>
#include <unistd.h>
#include <dirent.h>


#include "bmpfs.h"

// int is_dir(const char *path);
// int is_file(const char *path);
// void add_file(const char *filename);
//void add_dir(const char *dir_name);
void write_to_file(const char *path, const char *new_content);


char dir_list[256][256];
int curr_dir_idx = -1;

char files_list[256][256];
int curr_file_idx = -1;

char files_content[256][256];
int curr_file_content_idx = -1;

// typedef int (*fuse_fill_dir_t) (void *buf, const char *name, const struct stat *stbuf, off_t off);

static int do_getattr(const char *path, struct stat *st, struct fuse_file_info *fi) {
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = time( NULL );
	st->st_mtime = time( NULL );

	const char *file_name = bmp_get_filename();

	if (strcmp(path, "/") == 0) {
		st->st_mode = S_IFDIR| 0777;
		st->st_nlink = 2;
	}
	else if (file_name[0] != '\0' ) {
		st->st_mode = S_IFREG | 0777;
		st->st_nlink = 1;
		st->st_size = bmp_get_file_size();
	}
	else {
		return -ENOENT;
	}

	return 0;
}


static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags) {
	filler(buffer, ".", NULL, 0, 0);
	filler(buffer, "..", NULL, 0, 0);

	if (strcmp(path, "/") == 0) {
		filler(buffer, bmp_get_filename(), NULL, 0, 0);
	}

	return 0;
}


static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	const char *content = bmp_get_file_contents();

	memcpy(buffer, content + offset, size);
	return strlen(content) - offset;
}


void write_to_file(const char *path, const char *new_content) {
	bmp_set_file_contents(new_content);
}


static int do_write(const char *path, const char *buffer, size_t size, off_t offset, struct fuse_file_info *info) {
	printf("Writting content to file...\n");
	write_to_file(path, buffer);
	return size;
}


static int do_mknod(const char *path, mode_t mode, dev_t rdev) {
	path++;
	printf("Adding new file...\n");
	bmp_create_file(path);

	return 0;
}


static int do_unlink(const char *path) {
	//remove_file(path);
	printf("Deleting file...\n");
	bmp_delete_file();
	return 0;
}


static int do_rename(const char *path, const char *new_name, unsigned int flags) {
	printf("Renaming file...\n");
	path++; // Remove '/' from the beggining of path
	new_name++; // Remove '/' from the beginning of new_name
	bmp_set_filename(new_name);
	return 0;
}


void do_destroy(void *var) {
	printf("\nUnmounting...\n");
	bmp_close();
}


static const struct fuse_operations operations = {
	.getattr = do_getattr,
	.readdir = do_readdir,
	.mknod   = do_mknod,
	.write   = do_write,
	.read    = do_read,
	.unlink  = do_unlink,
	.rename  = do_rename,
	.destroy = do_destroy,
};


int create_folder(char *file_name, char *folder_name) {
	int result;
	int name_length = strlen(file_name) - 4; // Removing the extension from the file name 

	if (name_length < 1) {
		printf("\n** ERROR: you must provide a filename with an extension. E.g: myfile.bmp\n");
		return 1;
	}

	memcpy(folder_name, file_name, name_length);
	printf("Dir name: %s\n", folder_name);
	#ifdef __linux__
       result = mkdir(folder_name, 0777); 
   #else
       result = _mkdir(folder_name);
   #endif

	if (result == 1) {
		printf("\n** ERROR: Could not create folder %s\n", folder_name);
	}

   return result;
}


int main(int argc, char *argv[]) {
	if (strcmp(argv[1], "-h") == 0) {
		printf("\nTo mount a filesystem on a bitmap image:\n");
		printf("\t./secret-bit <filename.bmp>\n\n");
		return EXIT_SUCCESS;
	}

	char file_name[256] = {'\0'};
	char folder_name[128] = {'\0'};
	strcpy(file_name, argv[1]);

	int error = create_folder(file_name, folder_name);

	if (error == 1) {
		return EXIT_FAILURE;
	}

	printf("Loading file: %s\n", file_name);
	error = bmp_init(file_name);
	if (error) {
		return EXIT_FAILURE;
	}

	int f_argc = 6;
	char *f_argv[6];

	f_argv[0] = malloc(strlen(argv[0])+1);
	strcpy(f_argv[0], argv[0]);

	f_argv[1] = malloc(strlen("-s")+1);
	strcpy(f_argv[1], "-s");

	f_argv[2] = malloc(strlen("-f")+1);
	strcpy(f_argv[2], "-f");

	f_argv[3] = malloc(strlen("-o")+1);
	strcpy(f_argv[3], "-o");

	f_argv[4] = malloc(strlen("auto_unmount")+1);
	strcpy(f_argv[4], "auto_unmount");

	f_argv[5] = malloc(strlen(folder_name)+1);
	strcpy(f_argv[5], folder_name);


	printf("\n\n -> Secret Bit ready for use!\n");
    printf("\n-------------------------------\n");
    printf("    ** INSTRUCTIONS ** \n");
    printf("---------------------------------\n\n");
    printf("1. You can store a single text file into '%s' folder.\n", folder_name);
    printf("2. Allowed file operations: copy, delete and read the file contents.\n" );
    printf("3. The maximum available space is %ld byte(s). Any exceeding data will be discarded.\n", bmp_get_available_space() );
    printf("4. Unmount the folder or use ctrl+c to save its contents inside the bitmap image!\n");


	int result = fuse_main(f_argc, f_argv, &operations, NULL);
	
	free(f_argv[0]);
	free(f_argv[1]);
	free(f_argv[2]);
	free(f_argv[3]);
	free(f_argv[4]);
	free(f_argv[5]);
	
	return result;
}
