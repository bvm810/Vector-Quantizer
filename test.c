#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "vq.h"

char *generateOutputPath(char *ogName, char *dirName, unsigned K, unsigned W, unsigned H) {
    char *p, *outName, *treatedOgName = NULL;
    unsigned ogNameLen = 0, outNameLen;

    for (p = ogName; *p != '.'; p++) {
        ogNameLen += 1;
        treatedOgName = realloc(treatedOgName, ogNameLen);
        treatedOgName[ogNameLen - 1] = *p;
        if (*p == '/') {
            strncpy(treatedOgName, "", ogNameLen);
            ogNameLen = 0;
            free(treatedOgName);
            treatedOgName = NULL;
        }
    }
    ogNameLen += 1;
    treatedOgName = realloc(treatedOgName, ogNameLen);
    treatedOgName[ogNameLen - 1] = '\0';
    outNameLen = strlen(dirName) + strlen(treatedOgName) + 16 + (int) ceil(log10(K)) + (int) ceil(log10(W)) + (int) ceil(log10(H));
    outName = malloc(outNameLen);
    snprintf(outName, outNameLen, "%s/%s_K%i_W%i_H%i.pgm", dirName, treatedOgName, K, W, H);
    free(treatedOgName);
    return outName;
}

void logParamResults(Img *inImg, Img *outImg, unsigned K, unsigned width, unsigned height, char *logfileName) {
    FILE *fp;

    if ((fp = fopen(logfileName, "a")) == NULL)
        exit(LOG_ERROR);
    fprintf(fp, "%i,%i,%i,%f,%f\n", K, height, width, psnr(inImg, outImg), rate(K, height * width));
    fclose(fp);
}

void benchmarkParamsOnImage(char *imgName, char *outputDir, char *cbookName, unsigned cbookIdx, char *logfileName) {
    unsigned K, blockWidth, blockHeight, *idxList = NULL;
    char *outName = NULL;
    Cluster *clusters = NULL;
    Img *inImg = NULL, *outImg = NULL;
    BlockMatrix *vInImg = NULL, *vOutImg = NULL;

    clusters = readCodebook(cbookName, cbookIdx, &K, &blockWidth, &blockHeight);
    inImg = readPgmImg(imgName);
    vInImg = vectorizeImg(inImg, blockWidth, blockHeight);
    idxList = quantizeBlockMatrix(K, clusters, vInImg);
    vOutImg = reconstructBlockMatrix(clusters, idxList, blockHeight, blockWidth, getHeight(inImg), getWidth(inImg));
    outImg = deVectorizeImg(vOutImg);
    outName = generateOutputPath(imgName, outputDir, K, blockWidth, blockHeight);
    logParamResults(inImg, outImg, K, blockWidth, blockHeight, logfileName);
    writePgmImg(outImg, outName);
    freeClusters(clusters, K);
    free(idxList);
}

unsigned countCodebooks(char *codebookName) {
    unsigned ncodebooks = 0;
    char ch;
    FILE *fp;

    if ((fp = fopen(codebookName, "r")) == NULL)
        exit(INPUT_ERROR);
    ncodebooks = 0;
    while ((ch = (char) getc(fp)) != EOF)
        if (ch == '\n')
            ncodebooks += 1;
    fclose(fp);
    return ncodebooks;
}

int main (int argc, char **argv) {
    char **testImgNames;
    unsigned i, j, nfiles, ncodebooks;
    clock_t beginSingleBenchmark, endSingleBenchmark, beginBenchmark, endBenchmark;

    if (argc != 5)
        exit(INPUT_ERROR);
    ncodebooks = countCodebooks(argv[1]);
    testImgNames = pgmImgsInDir(argv[2], &nfiles);
    for (i = 0; i < nfiles; i++) {
        printf("Benchmark na imagem %s\n", testImgNames[i]);
        beginBenchmark = clock();
        for (j = 0; j < ncodebooks; j++) {
            printf("Codebook %i de %i ... ", j, ncodebooks);
            beginSingleBenchmark = clock();
            benchmarkParamsOnImage(testImgNames[i], argv[3], argv[1], j, argv[4]);
            endSingleBenchmark = clock();
            printf("tempo de execucao %f\n", (double) (endSingleBenchmark - beginSingleBenchmark)/CLOCKS_PER_SEC);
        }
        endBenchmark = clock();
        printf("Tempo de execucao para a imagem: %f\n", (double) (endBenchmark - beginBenchmark)/CLOCKS_PER_SEC);
    }
}