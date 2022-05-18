#include <stdlib.h>
#include "vq.h"

#define QUANTIZATION_ERROR 3

struct block {
    unsigned width;
    unsigned height;
    unsigned *vector;
};

struct blockMatrix {
    unsigned nrows;
    unsigned ncols;
    Block **blocks;
};

Block createBlock(unsigned **pixels, unsigned row, unsigned col, unsigned width, unsigned height) {
    Block *block;
    unsigned i, j;

    block = malloc(sizeof(Block) + width * height * sizeof(int));
    block->width = width;
    block->height = height;
    block->vector = malloc(width * height * sizeof(int));
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
            block->vector[width * i + j] = pixels[row + i][col + j];
    return *block;
}

Block **getImgBlocks(unsigned **pixels, unsigned nrows, unsigned ncols, unsigned blockWidth, unsigned blockHeight) {
    unsigned i, j;
    Block **blocks;

    blocks = malloc(nrows * sizeof(Block *));
    for (i = 0; i < nrows; i++) {
        blocks[i] = malloc(ncols * sizeof(Block));
        for (j = 0; j < ncols; j++)
            blocks[i][j] = createBlock(pixels, i * blockHeight, j * blockWidth, blockWidth, blockHeight);
    }
    return blocks;
}

BlockMatrix *vectorizeImg(Img *img, unsigned blockWidth, unsigned blockHeight) {
    unsigned imgWidth, imgHeight, **pixels;
    BlockMatrix *vectorizedImg;

    imgWidth = getWidth(img);
    imgHeight = getHeight(img);
    pixels = getPixels(img);
    if ((imgWidth % blockWidth != 0) || (imgHeight % blockHeight))
        exit(QUANTIZATION_ERROR);
    vectorizedImg = malloc(sizeof(BlockMatrix) + (imgHeight / blockHeight) * (imgWidth / blockWidth) * sizeof(Block));
    vectorizedImg->nrows = imgHeight / blockHeight;
    vectorizedImg->ncols = imgWidth / blockWidth;
    vectorizedImg->blocks = getImgBlocks(pixels, imgHeight / blockHeight, imgWidth / blockWidth, blockWidth, blockHeight);
    return vectorizedImg;
}

void testVQ(char *filename) {
    Img *img;
    BlockMatrix *vectorizedImg;

    img = readPgmImg(filename);
    vectorizedImg = vectorizeImg(img, 2, 2);
}