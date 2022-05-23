#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "vq.h"

#define INPUT_ERROR 7

void trainingCleanup(Img **imgs, BlockMatrix **vImgs, Cluster *clusters, Point *points, unsigned K, unsigned nImg) {
    unsigned i;

    for (i = 0; i < nImg; i++) {
        freeImg(imgs[i]);
        freeBlockMatrix(vImgs[i]);
    }
    free(imgs);
    free(vImgs);
    freeClusters(clusters, K);
    free(points);
}

void train(unsigned K, unsigned blockWidth, unsigned blockHeight, char *imgPath, int seed, char *cbookName, char *mode) {
    char **filenames;
    unsigned imgCount, nPoints;
    Point *points = NULL;
    Cluster *clusters = NULL;
    Img **imgs = NULL;
    BlockMatrix **vImgs = NULL;

    filenames = pgmImgsInDir(imgPath, &imgCount);
    imgs = readPgmImgs(filenames, imgCount);
    vImgs = vectorizeImgs(imgs, imgCount, blockWidth, blockHeight);
    points = pointsFromBlockMatrices(vImgs, imgCount, &nPoints);
    clusters = calculateCentroids(K, points, nPoints, seed);
    logCodebook(cbookName, mode, clusters, K, blockWidth, blockHeight);
    trainingCleanup(imgs, vImgs, clusters, points, K, imgCount);
}

int main (int argc, char **argv) {
    unsigned i, j, k;
    char imgPath[1000], codebookPath[1000];
    char *blockSizes[] = {"1x1", "1x2", "2x2", "2x4", "4x4", "8x4", "8x8"};
//    char *blockSizes[] = {"2x2","4x4"};
    unsigned Ks[] = {10, 50, 100, 200, 300, 400, 500, 600, 700, 800};
//    unsigned Ks[] = {200, 500};
    int seeds[] = {10, 20, 30};
//    int seeds[] = {10, 20};
    clock_t begin, end;

    if (argc != 3)
        exit(INPUT_ERROR);
    strcpy(imgPath, argv[1]);
    strcpy(codebookPath, argv[2]);
    for (i = 0; i < sizeof(blockSizes) / sizeof(char *); i++)
        for (j = 0; j < sizeof(Ks) / sizeof(int); j++)
            for (k = 0; k < sizeof(seeds) / sizeof(int); k++) {
                printf("Treinando params -  K: %i, W: %i, H: %i, seed: %i\n", Ks[j], (int) (blockSizes[i][2] - '0'), (int) (blockSizes[i][0] - '0'), seeds[k]);
                begin = clock();
                train(Ks[j], (int) (blockSizes[i][2] - '0'), (int) (blockSizes[i][0] - '0'), imgPath, seeds[k], codebookPath, "a");
                end = clock();
                printf("Tempo de execução do conjunto: %f\n", (double) (end - begin)/CLOCKS_PER_SEC);
            }
}