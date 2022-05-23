#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
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
            img->pixels[i][j] = 0;
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

Img **readPgmImgs(char **filenames, unsigned nImg) {
    Img **imgs;
    unsigned i;

    imgs = malloc(nImg * sizeof(Img *));
    for (i = 0; i < nImg; i++) {
        imgs[i] = readPgmImg(filenames[i]);
    }
    return imgs;
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

char **pgmImgsInDir(char *imgPath, unsigned *count) {
    DIR *imgDir;
    struct dirent *file;
    char *filename = NULL;
    char **imgsInDir = NULL;

    if ((imgDir = opendir(imgPath)) == NULL)
        exit(IMG_READ_ERROR);
    *count = 0;
    while ((file = readdir(imgDir)) != NULL) {
        if (file->d_type != DT_REG)
            continue;
        filename = malloc(strlen(imgPath) + file->d_namlen + 2);
        snprintf(filename, strlen(imgPath) + file->d_namlen + 2, "%s/%s", imgPath, file->d_name);
        *count += 1;
        imgsInDir = realloc(imgsInDir, *count * sizeof(char *));
        imgsInDir[*count-1] = filename;
        filename = NULL;
    }
    closedir(imgDir);
    return imgsInDir;
}

void testImgRead(char *imgPath) {
    unsigned count;
    char **files;

    files = pgmImgsInDir(imgPath, &count);
    for (int i = 0; i < count; i++)
        printf("Img %i: %s\n", i, files[i]);
}



