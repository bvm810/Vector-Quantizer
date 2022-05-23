#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "vq.h"

#define QUANTIZATION_ERROR 3
#define RECONSTRUCTION_ERROR 5

struct block {
    unsigned width;
    unsigned height;
    unsigned *vector;
};

struct blockMatrix {
    unsigned nrows;
    unsigned ncols;
    Block ***blocks;
};

Block *createBlockFromPixels(unsigned **pixels, unsigned row, unsigned col, unsigned width, unsigned height) {
    Block *block;
    unsigned i, j;

    block = malloc(sizeof(Block) + width * height * sizeof(int));
    block->width = width;
    block->height = height;
    block->vector = malloc(width * height * sizeof(int));
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
            block->vector[width * i + j] = pixels[row + i][col + j];
    return block;
}

Block *createBlockFromCluster(Cluster *cluster, unsigned width, unsigned height) {
    Block *block;
    unsigned i, j;

    block = malloc(sizeof(Block) + width * height * sizeof(int));
    block->width = width;
    block->height = height;
    block->vector = malloc(width * height * sizeof(int));
    for (i = 0; i < height; i++)
        for (j = 0; j < width; j++)
            block->vector[width * i + j] = getClusterCoords(cluster)[width * i + j];
    return block;
}

unsigned getBlockWidth(Block *b) {
    return b->width;
}

unsigned getBlockHeight(Block *b) {
    return b->height;
}

unsigned *getBlockVector(Block *b) {
    return b->vector;
}

void freeBlock(Block *block) {
    free(block->vector);
    free(block);
}

Block ***getImgBlocks(unsigned **pixels, unsigned nrows, unsigned ncols, unsigned blockWidth, unsigned blockHeight) {
    unsigned i, j;
    Block ***blocks;

    blocks = malloc(nrows * sizeof(Block **));
    for (i = 0; i < nrows; i++) {
        blocks[i] = malloc(ncols * sizeof(Block *));
        for (j = 0; j < ncols; j++)
            blocks[i][j] = createBlockFromPixels(pixels, i * blockHeight, j * blockWidth, blockWidth, blockHeight);
    }
    return blocks;
}

void freeBlockMatrix(BlockMatrix *vectorizedImg) {
    unsigned i, j;

    for (i = 0; i < vectorizedImg->nrows; i++) {
        for (j = 0; j < vectorizedImg->ncols; j++)
            freeBlock(vectorizedImg->blocks[i][j]);
        free(vectorizedImg->blocks[i]);
    }
    free(vectorizedImg->blocks);
    free(vectorizedImg);
}

BlockMatrix *vectorizeImg(Img *img, unsigned blockWidth, unsigned blockHeight) {
    unsigned imgWidth, imgHeight, **pixels;
    BlockMatrix *vectorizedImg;

    imgWidth = getWidth(img);
    imgHeight = getHeight(img);
    pixels = getPixels(img);
    if ((imgWidth % blockWidth != 0) || (imgHeight % blockHeight))
        exit(QUANTIZATION_ERROR);
    vectorizedImg = malloc(sizeof(BlockMatrix) + (imgHeight / blockHeight) * (imgWidth / blockWidth) * sizeof(Block *));
    vectorizedImg->nrows = imgHeight / blockHeight;
    vectorizedImg->ncols = imgWidth / blockWidth;
    vectorizedImg->blocks = getImgBlocks(pixels, imgHeight / blockHeight, imgWidth / blockWidth, blockWidth, blockHeight);
    return vectorizedImg;
}

BlockMatrix **vectorizeImgs(Img **imgs, unsigned nImgs, unsigned blockWidth, unsigned blockHeight) {
    unsigned i;
    BlockMatrix **vImgs;

    vImgs = malloc(nImgs * sizeof(BlockMatrix *));
    for (i = 0; i < nImgs; i++) {
        vImgs[i] = vectorizeImg(imgs[i], blockWidth, blockHeight);
    }
    return vImgs;
}

unsigned **generateImgPixels(BlockMatrix *vectorizedImg, unsigned imgWidth, unsigned imgHeight) {
    unsigned i, j;
    unsigned **pixels;
    unsigned blockWidth, blockHeight;

    blockWidth = vectorizedImg->blocks[0][0]->width;
    blockHeight = vectorizedImg->blocks[0][0]->height;
    pixels = malloc(imgHeight * sizeof(int *));
    for (i = 0; i < imgHeight; i++) {
        pixels[i] = malloc(imgWidth * sizeof(int));
        for (j = 0; j < imgWidth; j++) {
            pixels[i][j] = vectorizedImg->blocks[i / blockHeight][j / blockHeight]->vector[blockWidth * (i % blockHeight) + (j % blockWidth)];
        }
    }
    return pixels;
}

Img *deVectorizeImg(BlockMatrix *vectorizedImg) {
    unsigned imgWidth, imgHeight, **pixels;
    Img *img;

    // assumed all blocks are the same size in vectorized image
    imgWidth = vectorizedImg->ncols * vectorizedImg->blocks[0][0]->width;
    imgHeight = vectorizedImg->nrows * vectorizedImg->blocks[0][0]->height;
    pixels = generateImgPixels(vectorizedImg, imgWidth, imgHeight);
    // adapt here to accept images with mote than 8 bits/pixel
    img = createPgmImg(imgHeight, imgWidth, 1, pixels);
    freeBlockMatrix(vectorizedImg);
    return img;
}

unsigned getBlockMatrixHeight(BlockMatrix *vImg) {
    return vImg->nrows;
}

unsigned getBlockMatrixWidth(BlockMatrix *vImg) {
    return vImg->ncols;
}

Block ***getBlockMatrixBlocks(BlockMatrix *vImg) {
    return vImg->blocks;
}

unsigned *quantizeBlockMatrix(unsigned K, Cluster *clusters, BlockMatrix *vImg) {
    unsigned *idxList;
    Point *p;
    unsigned i, j;

    idxList = malloc(vImg->nrows * vImg->ncols * sizeof(int));
    for (i = 0; i < vImg->nrows; i++)
        for (j = 0; j < vImg->ncols; j++) {
            p = pointFromBlock(vImg->blocks[i][j]);
            assignCentroid(K, clusters, p);
            idxList[i * vImg->ncols + j] = getCluster(p);
        }
    return idxList;
}

BlockMatrix *reconstructBlockMatrix(Cluster *clusters, unsigned* idxList, unsigned blockHeight, unsigned blockWidth, unsigned imgHeight, unsigned imgWidth) {
    unsigned nrows, ncols;
    BlockMatrix *vImg;

    if ((imgWidth % blockWidth != 0) || (imgHeight % blockHeight))
        exit(RECONSTRUCTION_ERROR);
    nrows = imgHeight / blockHeight;
    ncols = imgWidth / blockWidth;
    vImg = malloc(sizeof(BlockMatrix) + (imgHeight / blockHeight) * (imgWidth / blockWidth) * sizeof(Block *));
    vImg->nrows = nrows;
    vImg->ncols = ncols;
    vImg->blocks = getIdxListBlocks(clusters, idxList, nrows, ncols, blockWidth, blockHeight);
    return vImg;
}

void testVQ(char *filenameInput, char *filenameOutput) {
    Img *ogImg, *outImg;
    BlockMatrix *vInImg, *vOutImg;
    Point *points;
    Cluster *clusters;
    unsigned K = 700, blockWidth = 2, blockHeight = 2, *idxList;
    clock_t begin, end;

    ogImg = readPgmImg(filenameInput);
    vInImg = vectorizeImg(ogImg, blockWidth, blockHeight);
    points = pointsFromBlockMatrix(vInImg);
    begin = clock();
    clusters = calculateCentroids(K, points, getBlockMatrixHeight(vInImg) * getBlockMatrixWidth(vInImg), 10);
    end = clock();
    printf("K: %i, Blocos: %i x %i, Pontos: %i, Tempo Execucao: %f", K, blockHeight, blockWidth, getBlockMatrixWidth(vInImg) *getBlockMatrixHeight(vInImg), (double) (end - begin) / CLOCKS_PER_SEC);
    idxList = quantizeBlockMatrix(K, clusters, vInImg);
    vOutImg = reconstructBlockMatrix(clusters, idxList, blockHeight, blockWidth, getHeight(ogImg), getWidth(ogImg));
    outImg = deVectorizeImg(vOutImg);
    writePgmImg(outImg, filenameOutput);
}