#ifndef __BMPFS_H
#define __BMPFS_H

typedef unsigned int int32;
typedef short int16;
typedef unsigned char byte;

int bmp_init(char *file_name);
void bmp_close(void);
void bmp_set_file_contents(const char *contents);
void bmp_set_filename(const char *filename);
void set_file_contents(const char *contents);
void bmp_delete_file(void);
void bmp_print_buffer(void);
void bmp_create_file(const char *filename);
const char *bmp_get_filename(void);
const char *bmp_get_file_contents(void);

size_t bmp_get_file_size(void);
size_t bmp_get_available_space(void);

#endif
