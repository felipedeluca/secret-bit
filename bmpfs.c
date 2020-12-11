/*
    Compile:
        gcc bmp.c -o bmp -lm

    References & Credits:
        Alejandro Rodriguez
        https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/

        GeekforGeeks:
        https://www.geeksforgeeks.org/modify-bit-given-position/
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>

#include "bmpfs.h"

#define DATA_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESSION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

#define FILE_NAME_LENGTH 256
#define MAX_USABLE_DATA_SIZE_RATIO 0.6 // 60%
//#define HEADER_NUM_BLOCKS 10 // Number of data blocks the head requires.
#define DATA_BLOCK_SIZE 8 // Size of a block in bytes.
#define BYTE_SIZE 8 // In bits
#define FILE_HEADER_SIZE 12

/* Set DEBUG to 1 if you want to see the pixels where the data was written */
#define DEBUG 0

char *getcwd(char *buf, size_t size);

char *header_file_system = "HEADER.FILE:"; // Head
// Buffer containing the stored data inside the BMP file.
char *stored_data_buffer;
// Max BMP space in bytes available to store data. 
size_t max_available_space;
// Max BMP space in bytes available to store data. 
size_t file_contents_available_space;
// Stored contents in BMP
char *file_contents;
char file_name[FILE_NAME_LENGTH];
char file_header[FILE_HEADER_SIZE];
char *image_name;

// Size of the space in bytes separating each block of BMP data.
size_t block_spacing;
// Size in bytes of bmp pixels.
size_t bmp_data_size;
// Number of blocks of BMP data of size DATA_BLOCK_SIZE.
size_t data_block_count;

/* BMP */
byte *pixels;
int32 width;
int32 height;
int32 bytes_per_pixel;
FILE *image_file;


/*
read_image
Modified source code. Original from:
Alejandro Rodriguez
        https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/
*/
int read_image(const char*filename, byte **pixels, int32 *width, int32 *height, int32 *bytes_per_pixel) {
    image_file = fopen(filename, "rb+");
    
    if (image_file == NULL) {
        return 1;
    }

    int32 data_offset;

    // Data offset
    fseek(image_file, DATA_OFFSET, SEEK_SET);
    fread(&data_offset, 4, 1, image_file);

    // Width
    fseek(image_file, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, image_file);

    // Height
    fseek(image_file, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, image_file);

    // Bits per pixel
    int16 bits_per_pixel;
    fseek(image_file, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bits_per_pixel, 2, 1, image_file);

    // Alocate a pixel array
    *bytes_per_pixel = ((int32) bits_per_pixel) / 8;

    // Each row is is padded to be  multiple of 4 bytes.
    int padded_row_size = (int) (4 * ceil((float)(*width / 4.0f)) * (*bytes_per_pixel));
    // Allocate memory for the pixel data
    int unpadded_row_size = (*width) * (*bytes_per_pixel);
    // Total size of the pixel data in bytes
    int total_size = unpadded_row_size * (*height);
    *pixels = (byte*) malloc(total_size);

    // Read the pixel data row by row.
    // Data is padded and stored botttom up.
    int i = 0;
    byte *current_row_pointer = *pixels + ((*height - 1) * unpadded_row_size);
    for (i = 0; i < *height; i++) {
        // put file cursor in the next row from top to bottom
        fseek(image_file, data_offset + (i * padded_row_size), SEEK_SET);
        // read only unpadded row size bytes 
        fread(current_row_pointer, 1, unpadded_row_size, image_file);
        // point to the next row (from bottom to top)
        current_row_pointer -= unpadded_row_size;
    }

    return 0;
}


/*
write_image
Modified source code. Original from:
Alejandro Rodriguez
        https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/
*/


void write_image(const char *file_name, byte *pixels, int32 width, int32 height, int32 bytes_per_pixel) {
    printf("\nOutput image: %s\n", file_name);

    rewind(image_file);
    // Open file in binary mode
    //FILE *image_file = fopen(file_name, "wb");

    if (image_file == NULL) {
        printf("\nError saving file!\n");
        return;
    }

    /* BMP Header */
    const char *bm = "BM";
    // write signature
    fwrite(&bm[0], 1, 1, image_file);
    fwrite(&bm[1], 1, 1, image_file);

    // write file size considering padded bytes;
    int padded_row_size = (int)(4 * ceil((float) width /4.0f)) * bytes_per_pixel;
    int32 file_size = padded_row_size * height + HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&file_size, 4, 1, image_file);

    // write reserved
    int32 reserved = 0x0000;
    fwrite(&reserved, 4, 1, image_file);

    // write data offset
    int32 data_offset = HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&data_offset, 4, 1, image_file);

    /* INFO + HEADER */
    // write size
    int32 info_header_size = INFO_HEADER_SIZE;
    fwrite(&info_header_size, 4, 1, image_file);

    // write width and height
    fwrite(&width, 4, 1, image_file);
    fwrite(&height, 4, 1, image_file);

    // write planes
    int16 planes = 1; // always 1
    fwrite(&planes, 2, 1, image_file);

    // write bits per pixel
    int16 bits_per_pixel = bytes_per_pixel * 8;
    fwrite(&bits_per_pixel, 2, 1, image_file);

    // write compression
    int32 compression = NO_COMPRESSION;
    fwrite(&compression, 4, 1, image_file);

    // write image size
    int32 image_size = width * height * bytes_per_pixel;
    fwrite(&image_size, 4, 1, image_file);

    // write resolution (pixels per meter).
    int32 resolutionX = 11811; // 300 dpi
    int32 resolutionY = 11811; // 300 dpi
    fwrite(&resolutionX, 4, 1, image_file);
    fwrite(&resolutionY, 4, 1, image_file);

    // write colors used
    int32 colors_used = MAX_NUMBER_OF_COLORS;
    fwrite(&colors_used, 4, 1, image_file);

    // write important colors
    int32 important_colors = ALL_COLORS_REQUIRED;
    fwrite(&important_colors, 4, 1, image_file);

    // Write data
    int i = 0;
    int unpadded_row_size = width * bytes_per_pixel;
    for (i = 0; i < height; i++) {
        // start writting from the beginning of last row in the pixel array
        int pixel_offset = ((height - i) -1) * unpadded_row_size;
        fwrite(&pixels[pixel_offset], 1, padded_row_size, image_file);
    }

    fclose(image_file);
    printf("\nData saved.\n");
}


void bmp_print_buffer(void) {
    printf("\nPrinting buffer...\n");
    for (size_t i = 0; i < max_available_space; i++) {
        printf("%c", stored_data_buffer[i]);
    }
    printf("\n\n");
}


void print_8bits(const char c) {
    int i = BYTE_SIZE - 1;
    for (; i >= 0; i--) {
        int bit = (c >> i) & 0x01;
        printf("%d", bit);
    }
    printf("\n");
}

/*
    Detect bit order.
*/
int get_least_significant_bit_index(void) {
    unsigned char c = 127;

    int bit = ((unsigned char)c >> 0) & 0x01;
    if (bit == 1) {
        if (DEBUG) {
            return 7;
        }
        return 0;
    }
    
    if (DEBUG) {
        return 0;
    }
    return 7;
}


int get_bit(char c, int pos) {
    return (c >> pos) & 0x01;
}


void store_bit(byte *dest, int bit, int bit_index) {
    int mask = 1 << bit_index;
    *dest = (*dest & ~mask) | ((bit << bit_index) & mask);
}


char read_block(byte *pixels, const char data_block, int block_idx) {
    int b_idx = get_least_significant_bit_index();

    int i = 0;
    unsigned char buffer = 'a';
    // Read stored bits in the data block
    for (; i < DATA_BLOCK_SIZE; i++) {
        int j = block_idx + i;
        int bit = get_bit(pixels[j], b_idx);
        store_bit(&buffer, bit, i);
    }
    return buffer;
}


void update_buffer(void) {
    memcpy(stored_data_buffer, file_header, FILE_HEADER_SIZE);
    memcpy(stored_data_buffer + FILE_HEADER_SIZE + 1, file_name, FILE_NAME_LENGTH);
    memcpy(stored_data_buffer + FILE_HEADER_SIZE + FILE_NAME_LENGTH + 1, file_contents, strlen(file_contents));
    printf("\nFile contents saved %ld byte(s)\n", strlen(file_contents));
}


void store_data(byte *pixels, const char *data) {
    int b_idx = get_least_significant_bit_index();
    int i = 0;
    int data_idx = 0;
    
    for (; i <= max_available_space; i += DATA_BLOCK_SIZE + block_spacing) {
        // Each char will be split into bits and each bit will be stored in the least significant bit of byte in a pixel.
        char d = data[data_idx];

        for (int j = 0; j < BYTE_SIZE; j++) {
            int bit = get_bit(d, j);
            store_bit(&pixels[i+j], bit, b_idx);
        }
        data_idx++;
    }
    return;
}


void load_file(void) {
    printf("Loading file...\n");
    memcpy(file_header, stored_data_buffer, FILE_HEADER_SIZE);
    printf("Head: %s\n", file_header);

    /* Check if there is a file stored */
    if (strcmp(file_header, header_file_system) != 0) {
        memset(file_header, 0, FILE_HEADER_SIZE);
        return;
    }

    printf("\t -> Head: %s\n", file_header);
    memcpy(file_name, stored_data_buffer + FILE_HEADER_SIZE + 1, FILE_NAME_LENGTH);
    memcpy(file_contents, stored_data_buffer + FILE_HEADER_SIZE + FILE_NAME_LENGTH + 1, file_contents_available_space);
    printf("\t -> File contents size: %ld\n", file_contents_available_space);
}


void read_data(byte *pixels) {
    int i = 0;
    char buffer = '0'; // 1 byte buffer
    int buffer_idx = 0;
    for (; i <= max_available_space; i += DATA_BLOCK_SIZE + block_spacing) {
        buffer = read_block(pixels, buffer, i);
        stored_data_buffer[buffer_idx] = buffer;
        buffer_idx++;
    }

    load_file();
}


void bmp_set_file_contents(const char *contents) {
    strcpy(file_contents, contents);
    printf("\nSetting file contents...\n");
}


const char *bmp_get_file_contents(void) {
    return file_contents;
}


void reset(void) {
    memset(stored_data_buffer, 0, max_available_space);
    memset(file_header, 0, FILE_HEADER_SIZE);
    memset(file_name, 0, FILE_NAME_LENGTH);

    file_contents = NULL;
    file_contents = (char *) malloc(file_contents_available_space);
    memset(file_contents, 0, file_contents_available_space);
}


void bmp_delete_file(void) {
    reset();
    printf("File deleted.\n");
}


int is_header_set(void) {
    if (strcmp(file_header, header_file_system) == 0)
        return 1;

    return 0;
}


void set_header() {
    strcpy(file_header, header_file_system);
}


void bmp_set_filename(const char *filename) {
    printf("\nSetting filename: |%s|\n", filename);
    strcpy(file_name, filename);
}


void bmp_create_file(const char *filename) {
    printf("Creating new file: |%s|", filename);
    reset();
    bmp_set_filename(filename);
    set_header();
}


size_t bmp_get_file_size(void) {
    return strlen(file_contents);
}


const char *get_filename(void) {
    return file_name;
}


const char *bmp_get_filename(void) {
    return get_filename();
}


void init_data(byte *pixels) {
    bmp_data_size = strlen((char *) pixels) * sizeof(unsigned char);
    max_available_space = (bmp_data_size * MAX_USABLE_DATA_SIZE_RATIO);
    file_contents_available_space = max_available_space - (FILE_HEADER_SIZE + FILE_NAME_LENGTH);
    printf("Available space: %zu bytes\n", file_contents_available_space);
    block_spacing = 0;
    size_t size = max_available_space * sizeof(char);
    stored_data_buffer = (char*) malloc(size);

    reset();
}


void save(void) {
    printf("\nSaving data...\n");
    update_buffer();
    store_data(pixels, stored_data_buffer);
    write_image(image_name, pixels, width, height, bytes_per_pixel);
 }


void bmp_close(void) {
    save();

    free(pixels);
    stored_data_buffer = NULL;

    free(stored_data_buffer);
    pixels = NULL;

    free(file_contents);
    file_contents = NULL;
}


size_t bmp_get_available_space(void) {
    return file_contents_available_space;
}


int bmp_init(char *fimage_name) {
    image_name = fimage_name;

    int error = read_image(image_name, &pixels, &width, &height, &bytes_per_pixel);
    if (error) {
        printf("\n** ERROR: could not read file '%s'\n\n", fimage_name);
        return 1;
    }

    init_data(pixels);

    // Read stored data within the BMP pixel data and saves on 'stored_data_buffer'.
    read_data(pixels);
    return 0;
}
