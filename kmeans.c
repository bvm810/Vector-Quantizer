#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "vq.h"

#define K_MEANS_ERROR 4

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

void initializeCentroids(int K, Point *points, unsigned nPoints, Cluster *newClusters, Cluster *oldClusters) {
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

char kMeansEndCondition(int K, Cluster *oldClusters, Cluster *newClusters) {
    int k, i;

    for (k = 0; k < K; k++)
        for (i = 0; i < newClusters[0].centroid.ndim; i++)
            if (oldClusters[k].centroid.coords[i] != newClusters[k].centroid.coords[i])
                return 0;
    return 1;
}

void addPointToCluster(Cluster *cluster, Point *point) {
    PointListElem *tmp, *new;

    cluster->count += 1;
    new = malloc(sizeof(PointListElem));
    new->next = NULL;
    new->point = point;
    if ((tmp = cluster->points) == NULL) {
        cluster->points = new;
        return;
    }
    while (tmp->next != NULL)
        tmp = tmp->next;
    tmp->next = new;
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

void assignCentroid(int K, Cluster *clusters, Point *point) {
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

void copyClusters(int K, Cluster *oldClusters, Cluster *newClusters) {
    int k, j;
    PointListElem *p;

    for (k = 0; k < K; k++) {
        // no need to cp points in cluster, they are never used in the old cluster
        // oldClusters[k].count = newClusters[k].count;
        // oldClusters[k].points = NULL;
        // for (p = newClusters[k].points; p != NULL; p = p->next)
        //     addPointToCluster(&oldClusters[k], p->point);
        for (j = 0; j < newClusters[k].centroid.ndim; j++)
            oldClusters[k].centroid.coords[j] = newClusters[k].centroid.coords[j];
    }
}

unsigned findLargestCluster(int K, Cluster *clusters) {
    int k;
    unsigned idx, max = 0;

    for (k = 0; k < K; k++)
        if (clusters[k].count > max) {
            max = clusters[k].count;
            idx = k;
        }
    return idx;
}

void replaceEmptyClusters(int K, Cluster *clusters) {
    unsigned i, k, largest;
    PointListElem *p;

    for (k = 0; k < K; k++)
        if (clusters[k].count == 0) {
            largest = findLargestCluster(K, clusters);
            p = clusters[largest].points;
            for (i = 0; i < rand() % clusters[largest].count; i++)
                p = p->next;
            addPointToCluster(&clusters[k], p->point);
            removePointFromCluster(&clusters[largest], p->point);
        }
}

void updateCentroids(int K, Cluster *clusters) {
    unsigned i, j, k;
    double *mean;
    PointListElem *p, *tmp;

    mean = calloc(clusters[0].centroid.ndim, sizeof(double));
    for (k = 0; k < K; k++) {
        p = clusters[k].points;
        tmp = p;
        for (i = 0; i < clusters[k].count; i++) {
            for (j = 0; j < clusters[k].centroid.ndim; j++) {
                mean[j] += (double) p->point->coords[j] / (double) clusters[k].count;
            }
            p = p->next;
            free(tmp);
            tmp = p;
        }
        free(p);
        clusters[k].points = NULL;
        clusters[k].count = 0;
        for (j = 0; j < clusters[k].centroid.ndim; j++) {
            clusters[k].centroid.coords[j] = (int) round(mean[j]);
            mean[j] = 0;
        }
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

void printClusters(int K, Cluster *clusters) {
    int i, k;

    for (k = 0; k < K; k++) {
        printf("Cluster %i: contagem - %i, coords - ", clusters[k].idx, clusters[k].count);
        for (i = 0; i < clusters[k].centroid.ndim; i++)
            printf("%i ", clusters[k].centroid.coords[i]);
        printf("\n");
    }
}

Cluster *calculateCentroids(int K, Point *points, unsigned nPoints, int seed) {
    Cluster *oldClusters, *newClusters;
    int i, iterationCounter = 0;
    clock_t begin, end;

    if (nPoints < K)
        exit(K_MEANS_ERROR);
    srand(seed);
    newClusters = malloc(K * (sizeof(Cluster) + points[0].ndim * sizeof(int)));
    oldClusters = malloc(K * (sizeof(Cluster) + points[0].ndim * sizeof(int)));
    initializeCentroids(K, points, nPoints, newClusters, oldClusters);
    printf("Inicializei centroides ...\n");
    // printClusters(K, newClusters);
    while (!kMeansEndCondition(K, oldClusters, newClusters)) {
        begin = clock();
        printf("Iteracao %i ... ", iterationCounter);
        fflush(stdout);
        for (i = 0; i < nPoints; i++)
            assignCentroid(K, newClusters, &points[i]);
        printf("Atribui centroides ... ");
        fflush(stdout);
        replaceEmptyClusters(K, newClusters);
        printf("Substitui centroides vazios... ");
        fflush(stdout);
        copyClusters(K, oldClusters, newClusters);
        printf("Copiei centroides novos ... ");
        fflush(stdout);
        // printClusters(K, oldClusters);
        updateCentroids(K, newClusters);
        printf("Atualizei centroides ... ");
        fflush(stdout);
        end = clock();
        printf("Tempo %f\n", (double) (end-begin) / CLOCKS_PER_SEC);
        iterationCounter++;

    }
    for (i = 0; i < K; i++)
        freeCluster(oldClusters[i]);
    free(oldClusters);
    return newClusters;
}

Point pointFromBlock(Block *b) {
    Point *p;

    p = malloc(sizeof(Point));
    p->cluster=0;
    p->ndim = getBlockHeight(b) * getBlockWidth(b);
    p->coords = getBlockVector(b);
    return *p;
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
            pArray[i * ncols + j] = pointFromBlock(blocks[i][j]);
    return pArray;
}



