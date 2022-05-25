#include <stdlib.h>
#include <stdio.h>
#include "vq.h"

int main (int argc, char **argv) {
    Cluster *clusters;
    unsigned K, blockWidth, blockHeight;

    if (argc != 3)
        exit(INPUT_ERROR);
    clusters = readCodebook(argv[2], 2, &K, &blockWidth, &blockHeight);
    printf("Params - K: %i, W: %i, H: %i\n", K, blockWidth, blockHeight);
    printClusters(K, clusters);
}