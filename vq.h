
typedef struct img Img;
typedef struct block Block;
typedef struct blockMatrix BlockMatrix;
typedef struct point Point;
typedef struct cluster Cluster;
typedef struct pointListElem PointListElem;

unsigned **getPixels(Img *img);
unsigned getWidth(Img *img);
unsigned getHeight(Img *img);
Img *readPgmImg(char *filename);
void writePgmImg(Img *img, char *filename);
void freeImg(Img *img);
Img *createPgmImg(unsigned height, unsigned width, unsigned pixelSize, unsigned **pixels);

unsigned getBlockWidth(Block *b);
unsigned getBlockHeight(Block *b);
unsigned *getBlockVector(Block *b);
unsigned getBlockMatrixHeight(BlockMatrix *vImg);
unsigned getBlockMatrixWidth(BlockMatrix *vImg);
Block ***getBlockMatrixBlocks(BlockMatrix *vImg);
Block *createBlockFromCluster(Cluster *cluster, unsigned width, unsigned height);

Cluster *calculateCentroids(unsigned K, Point *points, unsigned nPoints, int seed);
void assignCentroid(unsigned K, Cluster *clusters, Point *point);
Point *pointsFromBlockMatrix(BlockMatrix *vImg);
Point *pointFromBlock(Block *b);
unsigned getCluster(Point *p);
unsigned *getClusterCoords(Cluster *c);
Block ***getIdxListBlocks(Cluster *clusters, unsigned const *idxList, unsigned nrows, unsigned ncols, unsigned blockWidth, unsigned blockHeight);

// test functions - erase later
// void testImgRead(char *inputFilename, char *outputFilename);
void testVQ(char *filenameInput, char *filenameOutput);