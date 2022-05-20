#include <stdlib.h>
#include <math.h>
#include "vq.h"

#define K_MEANS_ERROR 4

struct point {
    unsigned cluster;
    unsigned ndim;
    unsigned *coords;
};

struct cluster {
    unsigned idx;
    unsigned count;
    unsigned *coords;
    Point *points;
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
    int i, idx;
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

unsigned **initializeCentroids(int K, Point *points, unsigned nPoints) {
    int i, j;
    unsigned **centroids, *prevIndexes = NULL, nPrev = 0;

    centroids = malloc(K * sizeof(int *));
    for (i = 0; i < K; i++) {
        centroids[i] = malloc(points[0].ndim * sizeof(int));
        for (j = 0; j < points[0].ndim; j++)
            centroids[i][j] = points[getUnseenRandIdx(prevIndexes, nPoints, &nPrev)].coords[j];
    }
    free(prevIndexes);
    return centroids;
}

char kMeansEndCondition(int K, unsigned **oldCentroids, unsigned **newCentroids, unsigned vectorSize) {
    int i, j;
    double oldComponentSum, newComponentSum;

    if (oldCentroids == NULL)
        return 0;
    for (i = 0; i < vectorSize; i++) {
        oldComponentSum = 0;
        newComponentSum = 0;
        for (j = 0; j < K; j++) {
            oldComponentSum += oldCentroids[j][i];
            newComponentSum += newCentroids[j][i];
        }
        if ((int) round(oldComponentSum / K) != (int) round(newComponentSum / K))
            return 0;
    }
    return 1;
}

void assignCentroid(int K, unsigned **centroids, Point point) {
    unsigned i;
    double minDist, dist;

    minDist = MAXFLOAT;
    for (i = 0; i < K; i++)
        if ((dist = distance(point.coords, centroids[i], point.ndim)) < minDist) {
            point.cluster = i;
            minDist = dist;
        }
}

void updateCentroids(unsigned **oldCentroids, unsigned **newCentroids, int K, Point *points, unsigned nPoints) {
    int i,j,k;
    double **sum;
    unsigned *counts;

    sum = malloc(K * sizeof(double *));
    for(k = 0; k < K; k++)
        sum[k] = calloc(points[0].ndim, sizeof(double));
    counts = calloc(K, sizeof(int));
    for (i = 0; i < nPoints; i++) {
        counts[points[i].cluster] += 1;
        for (j = 0; j < points[0].ndim; j++)
            sum[points[i].cluster][j] += points[i].coords[j];
    }
    for (k = 0; k < K; k++) {
        for (j = 0; j < points[0].ndim; j++) {
            oldCentroids[k][j] = newCentroids[k][j];
            if (counts[k] != 0)
                newCentroids[k][j] = (int) round(sum[k][j] / counts[k]);
            else
                newCentroids[k][j] = 0;
        }
    }
    for(k = 0; k < K; k++)
        free(sum[k]);
    free(sum);
    free(counts);
}

unsigned **calculateCentroids(int K, Point *points, unsigned nPoints, int seed) {
    unsigned **oldCentroids, **newCentroids;
    int i, j;

    if (nPoints > K)
        exit(K_MEANS_ERROR);
    srand(seed);
    oldCentroids = malloc(K * sizeof(int *));
    for (j = 0; j < K; j++)
        oldCentroids[j] = malloc(points[0].ndim * sizeof(int));
    newCentroids = initializeCentroids(K, points, nPoints);
    while (!kMeansEndCondition(K, oldCentroids, newCentroids, points[0].ndim)) {
        for (i = 0; i < nPoints; i++)
            assignCentroid(K, newCentroids, points[i]);
        updateCentroids(oldCentroids, newCentroids, K, points, nPoints);

    }
}

