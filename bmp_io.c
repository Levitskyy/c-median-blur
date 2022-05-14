/**
 * bmp_io.c -- функции чтения-записи файлов изображений в формате BMP
 *
 * Copyright (c) 2009, Student Name <student@cs.petrsu.ru>
 *
 * This code is licensed under a MIT-style license.
 */ 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include "bmp.h"

#define BMP_HEADER_SIZE 54
#define BMP_FILE_SIGNATURE 0x4D42

static void bmp_close(FILE* bmp_file, const char* bmp_path);

static void bmp_parse_header(bmp_header *header, const uint8_t *buf);

static void bmp_form_header(uint8_t *buf, const bmp_header *header);


bmp_image bmp_read(char *bmp_path)
{
    FILE *bmp_file;
    int i, j, k;
    
    uint8_t bmp_header_buf[BMP_HEADER_SIZE];
    
    bmp_image image;
    image.pixel_array = NULL; 

    /* Открываем файл с изображением и выводим диагностику в случае неудачи */
    if ((bmp_file = fopen(bmp_path, "r")) == NULL) {
        
        fprintf(stderr, "File \"%s\" couldn't be opened.\n%s\n",
                bmp_path, strerror(errno));
        
        exit(EXIT_FAILURE);
    }    
    
    /* Считываем файловый заголовок, выводим диагностику в случае неудачи */
    if (fread(bmp_header_buf, 1, BMP_HEADER_SIZE, bmp_file)
        < BMP_HEADER_SIZE) {
        
        int read_error = errno;
        
        /* Если файл слишком короткий, сообщаем об этом */
        if (feof(bmp_file)) {
            fprintf(stderr, "File \"%s\" is too short to be a BMP file.\n",
                bmp_path);
        } else if (ferror(bmp_file)) {
            fprintf(stderr, "File \"%s\" due to I/O error.\n", bmp_path);
            fprintf(stderr, "The reason is: %s\n", strerror(read_error));            
        }
        
        exit(EXIT_FAILURE);        
    }
    
    /* Разбираем файловый заголовок */
    bmp_parse_header(&image.header, bmp_header_buf);
    
    
    if (image.header.signature != BMP_FILE_SIGNATURE) {
        fprintf(stderr, "BMP signature doesn't match\n");
        exit(EXIT_FAILURE);
    }
    
    // Проверяем версию BMP (поддерживаются только заголовки BITMAPINFOHEADER)
    if (image.header.pixel_array_offset != BMP_HEADER_SIZE) {
        fprintf(stderr, "Unsupported BMP version\n");
        exit(EXIT_FAILURE);      
    }    
       
    // Проверяем глубину цвета (поддерживается только 24 бита на пиксель)
    if (image.header.bits_per_pixel != 24) {
        fprintf(stderr, "Unsupported BMP version\n");
        exit(EXIT_FAILURE);
    }    
    
    /* Считываем цвета точек в три различных цветовых плоскости */
    int size = image.header.width * image.header.height;
    image.pixel_array = (double*) malloc(3 * size * sizeof(double));

    if ((image.pixel_array) == NULL) {
        printf("Not enough memoy for reading picture.\n\n");
        exit(EXIT_FAILURE);
    }

    /* Переходим к считыванию цветов пикселей */
    fseek(bmp_file, BMP_HEADER_SIZE, SEEK_SET);

    /* Считываем значения цветовых компонентов, преобразуя к виду
       с плавающей точкой */
    for(i = 0; i < image.header.height; i++) {
        for (j = 0; j < image.header.width; j++) {
            double color;
            // Считываем значение синего компонента цвета пикселя
            color = (double) fgetc (bmp_file) / 255;
            image.pixel_array[i * image.header.width + j] = color;
            // Считываем значение зеленего компонента цвета пикселя
            color = (double) fgetc (bmp_file) / 255;            
            image.pixel_array[i * image.header.width + size + j] = color;
            // Считываем значение красного компонента цвета пикселя
            color = (double) fgetc (bmp_file) / 255;            
            image.pixel_array[i * image.header.width + 2 * size + j] = color;
        }
        
        // Пропускаем выравнивающие байты
        if (image.header.width * 3 % 4 != 0) {
            for (k = 0; k < 4 - image.header.width * 3 % 4; k++) {
                fgetc(bmp_file);
            }
        }
    }
    
    /* Закрываем файл с изображением и выводим диагностику в случае неудачи */
    bmp_close(bmp_file, bmp_path);
    
    return image;
}




void bmp_write(char *bmp_path, bmp_image image)
{
    FILE *bmp_file;
    int i, j, k;
    
    uint8_t bmp_header_buf[BMP_HEADER_SIZE];
    
    /* Открываем файл и выводим диагностику в случае неудачи */
    if ((bmp_file = fopen(bmp_path, "w")) == NULL) {
        
        fprintf(stderr, "File \"%s\" couldn't be opened.\n%s\n",
                bmp_path, strerror(errno));
        
        exit(EXIT_FAILURE);
    }    

    /* Формируем файловый заголовок на основе структуры */
    bmp_form_header(bmp_header_buf, &image.header);    
    
    /* Записываем файловый заголовок, выводим диагностику в случае неудачи */
    if (fwrite(bmp_header_buf, 1, BMP_HEADER_SIZE, bmp_file) < BMP_HEADER_SIZE) {
        
        int write_error = errno;
        
        /* Если файл слишком короткий, сообщаем об этом */
        if (ferror(bmp_file)) {
            fprintf(stderr, "File \"%s\" due to I/O error.\n", bmp_path);
            fprintf(stderr, "The reason is: %s\n", strerror(write_error));            
        }
        
        exit(EXIT_FAILURE);        
    }
    
    /* Записываем цвета точек в три различных цветовых плоскости */
    int size = image.header.width * image.header.height;

    /* Переходим к считыванию цветов пикселей */
    fseek(bmp_file, BMP_HEADER_SIZE, SEEK_SET);

    /* Записываем значения цветовых компонентов, преобразуя к виду
       с плавающей точкой */
    for(i = 0; i < image.header.height; i++) {
        for (j = 0; j < image.header.width; j++) {
            double color;
            // Считываем значение синего компонента цвета пикселя
            color = image.pixel_array[i * image.header.width + j];
            fputc((uint8_t)(color * 255), bmp_file);
            // Считываем значение зеленего компонента цвета пикселя          
            color = image.pixel_array[i * image.header.width + size + j];
            fputc((uint8_t)(color * 255), bmp_file);
            // Считываем значение красного компонента цвета пикселя          
            color = image.pixel_array[i * image.header.width + 2 * size + j];
            fputc((uint8_t)(color * 255), bmp_file);
        }
        
        // Пропускаем выравнивающие байты
        if (image.header.width * 3 % 4 != 0) {
            for (k = 0; k < 4 - image.header.width * 3 % 4; k++) {
                fputc(0, bmp_file);
            }
        }
    }
    
    /* Закрываем файл с изображением и выводим диагностику в случае неудачи */
    bmp_close(bmp_file, bmp_path);
    
    free(image.pixel_array);
}


/**
 * @brief извлекает данные заголовков BMP-файла
 */
static void bmp_parse_header(bmp_header *header, const uint8_t *buf)
{
    // Заполняем структуру данными файлового заголовка
    header->signature = *(uint16_t *)(buf + 0);
    header->file_size = *(uint32_t *)(buf + 2);
    header->reserved1 = *(uint16_t *)(buf + 6);
    header->reserved2 = *(uint16_t *)(buf + 8);
    header->pixel_array_offset = *(uint32_t *)(buf + 10);
    
    // Заполняем структуру данными DIB-заголовка
    header->header_size = *(uint32_t *)(buf + 14);
    header->width = *(int32_t *)(buf + 18);
    header->height = *(int32_t *)(buf + 22);
    header->color_planes = *(uint16_t *)(buf + 26);
    header->bits_per_pixel = *(uint16_t *)(buf + 28);
    header->compression_method = *(uint32_t *)(buf + 30);
    header->image_size = *(uint32_t *)(buf + 34);
    header->horizontal_resolution = *(int32_t *)(buf + 38);
    header->vertical_resolution = *(int32_t *)(buf + 42);
    header->colors = *(uint32_t *)(buf + 46);
    header->important_colors = *(uint32_t *)(buf + 50);    
}


/**
 * @brief подготавливает заголовок BMP-файла
 */
static void bmp_form_header(uint8_t *buf, const bmp_header *header)
{
    // Заполняем файловый заголовок
    *(uint16_t *)(buf + 0) = header->signature;
    *(uint32_t *)(buf + 2) = header->file_size;
    *(uint16_t *)(buf + 6) = header->reserved1;
    *(uint16_t *)(buf + 8) = header->reserved2;
    *(uint32_t *)(buf + 10) = header->pixel_array_offset;

    // Заполняем информационные заголовок
    *(uint32_t *)(buf + 14) = header->header_size;
    *(int32_t *)(buf + 18) = header->width;
    *(int32_t *)(buf + 22) = header->height;
    *(uint16_t *)(buf + 26) = header->color_planes;
    *(uint16_t *)(buf + 28) = header->bits_per_pixel;
    *(uint32_t *)(buf + 30) = header->compression_method;
    *(uint32_t *)(buf + 34) = header->image_size;
    *(int32_t *)(buf + 38) = header->horizontal_resolution;
    *(int32_t *)(buf + 42) = header->vertical_resolution;
    *(uint32_t *)(buf + 46) = header->colors;
    *(uint32_t *)(buf + 50) = header->important_colors;
}


/**
 * @brief закрывает файл с изображением, контролируя ошибки
 */
static void bmp_close(FILE* bmp_file, const char* bmp_path)
{
    /* Закрываем файл с изображением */
    if (fclose(bmp_file) != EOF) 
        return; 
        
    /* Выводим диагностику в случае неудачной попытки закрытия файла */
    int close_error = errno;
        
    fprintf(stderr, "File %s couldn't be closed appropriately\n", bmp_path);
    fprintf(stderr, "The reason is: %s\n", strerror(close_error));
        
    exit(EXIT_FAILURE);
}


