#include <stdlib.h>
#include <stdio.h>
#include "vq.h"

#define IMG_READ_ERROR 1
#define IMG_WRITE_ERROR 2

struct img {
    unsigned height;
    unsigned width;
    unsigned pixelSize;
    unsigned **pixels;
};

char *readPgmHeaderElement(FILE * fp, char stopChar) {
    char *buffer, ch;
    int i = 0;

    buffer = NULL;
    while ((ch = (char) getc(fp)) != stopChar) {
        buffer = realloc(buffer, 1+i);
        *(buffer + i) = ch;
        i++;
    }
    return buffer;
}

void readPgmHeader(FILE *fp, unsigned *width, unsigned *height, unsigned *pixelSize) {
    char *endptr;

    // adapt this to better fit the PGM specification later ...
    readPgmHeaderElement(fp, '\n');
    *width = (unsigned int) strtol(readPgmHeaderElement(fp, ' '), &endptr, 10);
    *height = (unsigned int) strtol(readPgmHeaderElement(fp, '\n'), &endptr, 10);
    *pixelSize = (unsigned int) strtol(readPgmHeaderElement(fp, '\n'), &endptr, 10);
}

void readPgmPixels(FILE *fp, Img *img) {
    unsigned i,j;

    // assumed only 8-bit images, change this to adapt to pgm format later on
    img->pixels = malloc(img->height * sizeof(int*));
    for (i = 0; i < img->height; i++) {
        img->pixels[i] = malloc(img->width * sizeof(int));
        for (j = 0; j < img->width; j++) {
            if (fread(&img->pixels[i][j], 1, 1, fp) != 1)
                exit(IMG_READ_ERROR);
        }
    }
}

Img *readPgmImg(char *filename) {
    unsigned width, height, pixelSize;
    FILE *fp;
    Img *img;

    if ((fp = fopen(filename, "rb")) == NULL)
        return NULL;
    readPgmHeader(fp, &width, &height, &pixelSize);
    img = malloc(sizeof(Img) + width * height * sizeof(int));
    img->width = width;
    img->height = height;
    img->pixelSize = pixelSize;
    readPgmPixels(fp, img);
    fclose(fp);
    return img;
}

void freeImg(Img *img) {
    unsigned i;

    for (i = 0; i < img->height; i++)
        free(img->pixels[i]);
    free(img->pixels);
    free(img);
}

void writePgmImg(Img *img, char *filename) {
    FILE *fp;
    unsigned i, j;

    if ((fp = fopen(filename, "wb")) == NULL)
        exit(IMG_WRITE_ERROR);
    fprintf(fp, "P5\n%i %i\n%i\n", img->width, img->height, img->pixelSize);
    // assumed only 8-bit images, change this to adapt to pgm format later on
    for (i = 0; i < img->height; i++)
        for (j = 0; j < img->width; j++)
            if (fwrite(&img->pixels[i][j], 1, 1, fp) != 1)
                exit(IMG_WRITE_ERROR);
    fclose(fp);
    freeImg(img);
}

Img *createPgmImg(unsigned height, unsigned width, unsigned pixelSize, unsigned **pixels) {
    Img *img;

    img = malloc(sizeof(Img) + height * width * sizeof(int));
    img->width = width;
    img->height = height;
    img->pixelSize = pixelSize;
    img->pixels = pixels;
    return img;
}

unsigned **getPixels(Img *img) {
    return img->pixels;
}

unsigned getWidth(Img *img) {
    return img->width;
}

unsigned getHeight(Img *img) {
    return img->height;
}

/** imgread test functions - erase later
void testImgRead(char *inputFilename, char *outputFilename) {
    Img *img;

    img = readPgmImg(inputFilename);
    writePgmImg(img, outputFilename);
}

**/



