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

int main(int argc, char *argv[])
{
    bmp_image image;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s sourcefile destfile\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    image = bmp_read(argv[1]);

    median_blur(&image);

    bmp_write(argv[2], image);

    return 0;
}
