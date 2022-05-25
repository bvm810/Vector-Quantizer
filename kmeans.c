#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "vq.h"

#define K_MEANS_ERROR 4
#define LOG_ERROR 6

struct point {
    unsigned cluster;
    unsigned ndim;
    unsigned *coords;
};

struct pointListElem {
    struct pointListElem *next;
    Point *point;
};

struct cluster {
    unsigned idx;
    unsigned count;
    Point centroid;
    struct pointListElem *points;
};

// used unsigned here because pixel values will always be unsigned ints
double distance(unsigned const *vectorOne, unsigned const *vectorTwo, unsigned vectorSize) {
    int i;
    double sum = 0;

    for (i = 0; i < vectorSize; i++)
        sum += (vectorOne[i] - vectorTwo[i]) * (vectorOne[i] - vectorTwo[i]);
    return sqrt(sum);
}

unsigned getUnseenRandIdx(unsigned *prevIndexes, unsigned nPoints, unsigned *nPrev) {
    int i;
    unsigned idx;
    char hasBeenSeen = 1;


    prevIndexes = realloc(prevIndexes, *nPrev+sizeof(int));
    while (hasBeenSeen) {
        idx = rand() % nPoints;
        hasBeenSeen = 0;
        for (i = 0; i < (sizeof(prevIndexes) / sizeof(int)) - 1; i++)
            if (prevIndexes[i] == idx)
                hasBeenSeen = 1;
    }
    prevIndexes[i+1] = idx;
    *nPrev = *nPrev + 1;
    return idx;
}

void initializeCentroids(unsigned K, Point *points, unsigned nPoints, Cluster *newClusters, Cluster *oldClusters) {
    int i, j;
    Point p;
    unsigned *prevIndexes = NULL, nPrev = 0;

    for (i = 0; i < K; i++) {
        newClusters[i] = (Cluster) {i, 0, {i, points[0].ndim, NULL}, NULL};
        oldClusters[i] = (Cluster) {i, 0, {i, points[0].ndim, NULL}, NULL};
        newClusters[i].centroid.coords = malloc(newClusters[i].centroid.ndim * sizeof(int));
        oldClusters[i].centroid.coords = malloc(oldClusters[i].centroid.ndim * sizeof(int));
        p = points[getUnseenRandIdx(prevIndexes, nPoints, &nPrev)];
        for (j = 0; j < points[0].ndim; j++)
            newClusters[i].centroid.coords[j] = p.coords[j];
    }
    free(prevIndexes);
}

char kMeansEndCondition(unsigned K, Cluster *oldClusters, Cluster *newClusters) {
    int k, i;
    double err;

    for (k = 0; k < K; k++) {
        err = 0;
        for (i = 0; i < newClusters[0].centroid.ndim; i++)
            err += (double) (oldClusters[k].centroid.coords[i] - newClusters[k].centroid.coords[i]);
        if (err/(double) newClusters[0].centroid.ndim > 12.75)
            return 0;
    }
    return 1;
}

void addPointToCluster(Cluster *cluster, Point *point) {
    PointListElem *tmp, *new;

    cluster->count += 1;
    new = malloc(sizeof(PointListElem));
    new->point = point;
    tmp = cluster->points;
    new->next = tmp;
    cluster->points = new;
}

void removePointFromCluster(Cluster *cluster, Point *point) {
    PointListElem *tmp, *aux;

    cluster->count -= 1;
    if ((tmp = cluster->points) == NULL)
        exit(K_MEANS_ERROR);
    if (tmp->point == point) {
        cluster->points = tmp->next;
        free(tmp);
        return;
    }
    while ((tmp->next != NULL) && (tmp->next->point != point))
        tmp = tmp->next;
    if (tmp->next == NULL)
        exit(K_MEANS_ERROR);
    aux = tmp->next;
    tmp->next = aux->next;
    free(aux);
}

void assignCentroid(unsigned K, Cluster *clusters, Point *point) {
    unsigned k, clusterIdx;
    double minDist, dist;

    minDist = MAXFLOAT;
    for (k = 0; k < K; k++)
        if ((dist = distance(point->coords, clusters[k].centroid.coords, point->ndim)) < minDist) {
            clusterIdx = k;
            minDist = dist;
        }
    point->cluster = clusterIdx;
    addPointToCluster(&clusters[clusterIdx], point);
}

void copyClusters(unsigned K, Cluster *oldClusters, Cluster *newClusters) {
    int k, j;

    for (k = 0; k < K; k++) {
        // no need to cp points in cluster, they are never used in the old cluster
        for (j = 0; j < newClusters[k].centroid.ndim; j++)
            oldClusters[k].centroid.coords[j] = newClusters[k].centroid.coords[j];
    }
}

unsigned findLargestCluster(unsigned K, Cluster *clusters) {
    int k;
    unsigned idx, max = 0;

    for (k = 0; k < K; k++)
        if (clusters[k].count > max) {
            max = clusters[k].count;
            idx = k;
        }
    return idx;
}

void replaceEmptyClusters(unsigned K, Cluster *clusters) {
    unsigned k, largest;
    PointListElem *p;

    for (k = 0; k < K; k++)
        if (clusters[k].count == 0) {
            largest = findLargestCluster(K, clusters);
            p = clusters[largest].points;
            addPointToCluster(&clusters[k], p->point);
            removePointFromCluster(&clusters[largest], p->point);
        }
}

void updateCentroids(unsigned K, Cluster *clusters) {
    unsigned i, j, k;
    double *mean;
    PointListElem *p, *tmp;

    mean = calloc(clusters[0].centroid.ndim, sizeof(double));
    for (k = 0; k < K; k++) {
        p = clusters[k].points;
        tmp = p;
        for (i = 0; i < clusters[k].count; i++) {
            for (j = 0; j < clusters[k].centroid.ndim; j++) {
                mean[j] += (double) p->point->coords[j];
            }
            p = p->next;
            free(tmp);
            tmp = p;
        }
        free(p);
        clusters[k].points = NULL;
        for (j = 0; j < clusters[k].centroid.ndim; j++) {
            clusters[k].centroid.coords[j] = (int) round(mean[j] / clusters[k].count);
            mean[j] = 0;
        }
        clusters[k].count = 0;
    }
    free(mean);
}

void freeCluster(Cluster cluster) {
    PointListElem *p, *next;

    if ((p = cluster.points) != NULL) {
        next = p->next;
        while (next != NULL) {
            free(p);
            p = next;
            next = p->next;
        }
    }
    free(cluster.centroid.coords);
}

void freeClusters(Cluster *clusters, unsigned K) {
    unsigned i;

    for (i = 0; i < K; i++)
        freeCluster(clusters[i]);
    free(clusters);
}

void printClusters(unsigned K, Cluster *clusters) {
    int i, k;

    for (k = 0; k < K; k++) {
        printf("Cluster %i: contagem - %i, coords - ", clusters[k].idx, clusters[k].count);
        for (i = 0; i < clusters[k].centroid.ndim; i++)
            printf("%i ", clusters[k].centroid.coords[i]);
        printf("\n");
    }
}

Cluster *calculateCentroids(unsigned K, Point *points, unsigned nPoints, int seed) {
    Cluster *oldClusters, *newClusters;
    int i, counter = 1;
    clock_t begin, end;

    if (nPoints < K)
        exit(K_MEANS_ERROR);
    srand(seed);
    newClusters = malloc(K * (sizeof(Cluster) + points[0].ndim * sizeof(int)));
    oldClusters = malloc(K * (sizeof(Cluster) + points[0].ndim * sizeof(int)));
    initializeCentroids(K, points, nPoints, newClusters, oldClusters);
    while (!kMeansEndCondition(K, oldClusters, newClusters) && (counter < 41)) {
        // printClusters(K, newClusters);
        // printf("Iteracao %i ... ", counter);
        // fflush(stdout);
        begin = clock();
        for (i = 0; i < nPoints; i++)
            assignCentroid(K, newClusters, &points[i]);
        replaceEmptyClusters(K, newClusters);
        copyClusters(K, oldClusters, newClusters);
        updateCentroids(K, newClusters);
        end = clock();
        // printf("tempo de execucao %f\n", (double) (end - begin)/CLOCKS_PER_SEC);
        counter++;
    }
    for (i = 0; i < K; i++)
        freeCluster(oldClusters[i]);
    free(oldClusters);
    return newClusters;
}

Point *pointFromBlock(Block *b) {
    Point *p;

    p = malloc(sizeof(Point));
    p->cluster=0;
    p->ndim = getBlockHeight(b) * getBlockWidth(b);
    p->coords = getBlockVector(b);
    return p;
}

Point *pointsFromBlockMatrix(BlockMatrix *vImg) {
    unsigned i, j, nrows, ncols;
    Block ***blocks;
    Point *pArray;

    nrows = getBlockMatrixHeight(vImg);
    ncols = getBlockMatrixWidth(vImg);
    blocks = getBlockMatrixBlocks(vImg);
    pArray = malloc(getBlockMatrixWidth(vImg) * getBlockMatrixHeight(vImg) * sizeof(Point));
    for (i = 0; i < nrows; i++)
        for (j = 0; j < ncols; j++)
            pArray[i * ncols + j] = (Point) {0, getBlockHeight(blocks[i][j]) * getBlockWidth(blocks[i][j]), getBlockVector(blocks[i][j])};
    return pArray;
}

Point *pointsFromBlockMatrices(BlockMatrix **vImgs, unsigned nImgs, unsigned *nPoints) {
    Point *points = NULL;
    Block ***blocks, *b;
    unsigned i, j, k, nrows, ncols;

    *nPoints = 0;
    for (i = 0; i < nImgs; i++) {
        nrows = getBlockMatrixHeight(vImgs[i]);
        ncols = getBlockMatrixWidth(vImgs[i]);
        points = realloc(points, (*nPoints + nrows * ncols) * sizeof(Point));
        blocks = getBlockMatrixBlocks(vImgs[i]);
        for (j = 0; j < nrows; j++)
            for (k = 0; k < ncols; k++) {
                b = blocks[j][k];
                points[*nPoints + j * ncols + k] = (Point) {0,getBlockHeight(b) * getBlockWidth(b),getBlockVector(b)};
            }
        *nPoints = *nPoints + nrows * ncols;
    }
    return points;
}

unsigned getCluster(Point *p) {
    return p->cluster;
}

unsigned *getClusterCoords(Cluster *c) {
    return c->centroid.coords;
}

Block ***getIdxListBlocks(Cluster *clusters, unsigned const *idxList, unsigned nrows, unsigned ncols, unsigned blockWidth, unsigned blockHeight) {
    Block ***blocks;
    unsigned i, j;

    blocks = malloc(nrows * sizeof(Block **));
    for (i = 0; i < nrows; i++) {
        blocks[i] = malloc(ncols * sizeof(Block *));
        for (j = 0; j < ncols; j++) {
            blocks[i][j] = createBlockFromCluster(&clusters[idxList[i * ncols + j]], blockWidth, blockHeight);
        }
    }
    return blocks;
}

void logCodebook(char *codebookFilename, char *mode, Cluster *clusters, unsigned K, unsigned blockWidth, unsigned blockHeight) {
    FILE *fp;
    unsigned i, j;

    if ((fp = fopen(codebookFilename, mode)) == NULL)
        exit(LOG_ERROR);
    fprintf(fp, "%i,%i,%i,", K, blockHeight, blockWidth);
    for (i = 0; i < K; i++) {
        fprintf(fp, "%i,", clusters[i].idx);
        for (j = 0; j < blockWidth * blockHeight; j++) {
            fprintf(fp, "%i", clusters[i].centroid.coords[j]);
            if ((i != K - 1) || (j != blockWidth * blockHeight - 1))
                fprintf(fp, ",");
        }
    }
    fprintf(fp, ";\n");
    fclose(fp);
}

char *readCodebookInfo(FILE *fp, char stopChar) {
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

Cluster *readCodebookCluster(FILE *fp, unsigned ndim, unsigned K) {
    Cluster * clusters;
    char stopChar, ch;
    unsigned i, j, idx;

    clusters = malloc(K * (sizeof(Cluster) + ndim * sizeof(int)));
    for (i = 0; i < K; i++) {
        idx = (unsigned int) strtol(readCodebookInfo(fp, ','), NULL, 10);
        clusters[i] = (Cluster) {idx, 0, {idx, ndim, NULL}, NULL};
        clusters[i].centroid.coords = malloc(ndim * sizeof(int));
        for (j = 0; j < ndim; j++) {
            stopChar = ((i == K - 1) && (j == ndim -1)) ? ';' : ',';
            clusters[i].centroid.coords[j] = (unsigned int) strtol(readCodebookInfo(fp, stopChar), NULL, 10);
        }
    }
    return clusters;
}

Cluster *readCodebook(char *filename, unsigned id, unsigned *K, unsigned *blockWidth, unsigned *blockHeight) {
    FILE *fp;
    char ch = '0';
    Cluster *clusters;
    unsigned ndim, line = 0;

    if ((fp = fopen(filename, "r")) == NULL )
        exit(LOG_ERROR);
    while ((line < id) && (ch != EOF)) {
        while ((ch != '\n') && (ch != EOF))
            ch = (char) getc(fp);
        line++;
    }
    *K = (unsigned int) strtol(readCodebookInfo(fp, ','), NULL, 10);
    *blockHeight = (unsigned int) strtol(readCodebookInfo(fp, ','), NULL, 10);
    *blockWidth = (unsigned int) strtol(readCodebookInfo(fp, ','), NULL, 10);
    ndim = (*blockHeight) * (*blockWidth);
    clusters = readCodebookCluster(fp, ndim, *K);
    fclose(fp);
    return clusters;
}



