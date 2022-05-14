#ifndef BMP_H
#define BMP_H

#include <stdint.h>

/* Структура заголовка BMP-файла */
typedef struct _bmp_header {
    
    /* Данные файлового заголовка */
    uint16_t signature;
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t pixel_array_offset;
    
    /* Данные информационного заголовка */
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t color_planes;
    uint16_t bits_per_pixel;
    uint32_t compression_method;
    uint32_t image_size;
    int32_t horizontal_resolution;
    int32_t vertical_resolution;
    uint32_t colors;
    uint32_t important_colors;    
} bmp_header;

typedef double *bmp_pixel_array;

typedef struct _bmp_image {
    bmp_header header;
    bmp_pixel_array pixel_array;
} bmp_image;

bmp_image bmp_read(char *bmp_file_path);

void bmp_write(char *bmp_file_path, bmp_image image);

void bmp_gray(bmp_image image);

void median_blur(bmp_image *image);

#endif
