/**
 * filter.c -- программа для наложения фильтров на
 * изображение в формате BMP
 *
 * Copyright (c) 2021, Levickii Ivan <levickii@cs.petrsu.ru>
 *
 * This code is licensed under a MIT-style license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "bmp.h"

/* Поиск медианы в массиве  */
double find_median(double *pixel_array);

/* Определение цвета пикселя */
int get_color(double *pixel_array, bmp_image * image);

int cmp(const void *a, const void *b);

/* Создание массива для нахождения медианы  */
double *create_median_window(double *pixel_array, bmp_image * image);

/* Применение медианного фильтра */
void median_blur(bmp_image * image)
{
    /* Получаем указатель на массив пикселей */
    double *pixel_array = image->pixel_array;
    
    /* Получаем размер изображения */
    int size = image->header.height * image->header.width;
    /* Для каждого пикселя  */
    for (int h = 0; h < image->header.height; ++h) {
        for (int w = 0; w < image->header.width; ++w) {
            /* Получаем адрес пикселя для синего, зеленого и красного цвета */
            double *current_pixel_b =
                &pixel_array[h * image->header.width + 0 * size + w];
            double *current_pixel_g =
                &pixel_array[h * image->header.width + 1 * size + w];
            double *current_pixel_r =
                &pixel_array[h * image->header.width + 2 * size + w];
            /* Для каждого цвета создаем матрицу для поиска медианы */
            double *buf_b = create_median_window(current_pixel_b, image);
            double *buf_g = create_median_window(current_pixel_g, image);
            double *buf_r = create_median_window(current_pixel_r, image);
            /* Для каждого цвета находим медиану */
            *current_pixel_b = find_median(buf_b);
            *current_pixel_g = find_median(buf_g);
            *current_pixel_r = find_median(buf_r);
            free(buf_b);
            free(buf_g);
            free(buf_r);
        }
    }
}

int cmp(const void *a, const void *b)
{
    double d1 = *(double*)a;
    double d2 = *(double*)b;
    return
	    (d1 > d2) ? 1 :
	    (d1 < d2) ? -1 : 0;
}


double find_median(double *pixel_array)
{

    /* Сортируем значения пикселей по возрастанию  */
    qsort(pixel_array, 25, sizeof(double), cmp);

    /* Возвращаем медиану */
    return pixel_array[12]; 
}

double *create_median_window(double *pixel_array, bmp_image * image)
{
    /* Выделяем память для хранения матрицы 5x5 */
    double *buf = (double *) malloc(sizeof(double) * 25);
    /* Сохраняем адрес начала матрицы */
    double *start_buf = buf;
    /* Получаем размер изображения */
    int size = image->header.height * image->header.width;
    /* Получаем цвет центра матрицы  */
    int color = get_color(pixel_array, image);

    /* Для кажого элемента в матрице  */
    for (int h = 0; h < 5; ++h) {
        for (int w = 0; w < 5; ++w) {
            /* Получаем адрес позиции текущего элемента */
            double *current_pos =
                &pixel_array[(h - 2) * image->header.width + (w - 2)];
            /* Проверяем, не выходит ли он за пределы массива пикселей */
            if (current_pos >= image->pixel_array
                && current_pos < &(image->pixel_array[3 * size])) {
                /* Получаем цвет текущего элемента */
                int current_color = get_color(current_pos, image);
                if (current_color != color) {
                    /* Если цвет не совпадает, берем значения центрального элемента */
                    *buf = *pixel_array;
                } else {
                    /* Иначе берем значение текущего элемента  */
                    *buf = *current_pos;
                }
            } else 
                *buf = *pixel_array;
            buf++;
        }
    }

    return start_buf;
}

int get_color(double *pixel_array, bmp_image * image)
{
    int size = image->header.height * image->header.width;
    /* Синий цвет  */
    if (pixel_array < &(image->pixel_array[size])) {
        return 0;
    }
    /* Зеленый цвет  */
    if (pixel_array < &(image->pixel_array[2 * size])) {
        return 1;
    }
    /* Красный цвет */
    return 2;

}
