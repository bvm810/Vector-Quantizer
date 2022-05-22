#include <stdio.h>
#include <time.h>
#include "vq.h"

void train(unsigned K, unsigned blockWidth, unsigned blockHeight, char *imgPath, int seed) {
    char **filenames;
    unsigned imgCount, nPoints;
    Point *points;
    Cluster *clusters;
    BlockMatrix **vImgs;
    clock_t begin, end;
    double elapsed;


    filenames = pgmImgsInDir(imgPath, &imgCount);
    vImgs = vectorizeImgs(filenames, imgCount, blockWidth, blockHeight);
    points = pointsFromBlockMatrices(vImgs, imgCount, &nPoints);
    printf("Calculando clusters ... %i imagens, %i pontos, dimensão %i, %i clusters\n", imgCount, nPoints, blockHeight * blockWidth, K);
    begin = clock();
    clusters = calculateCentroids(K, points, nPoints, seed);
    end = clock();
    printf("Tempo de execução: %f", (double) (end - begin) / CLOCKS_PER_SEC);
    // funcao pra escrever os codebooks ...
}