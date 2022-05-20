
typedef struct img Img;
typedef struct block Block;
typedef struct blockMatrix BlockMatrix;
typedef struct point Point;
typedef struct cluster Cluster;

unsigned **getPixels(Img *img);
unsigned getWidth(Img *img);
unsigned getHeight(Img *img);
Img *readPgmImg(char *filename);
void writePgmImg(Img *img, char *filename);
void freeImg(Img *img);
Img *createPgmImg(unsigned height, unsigned width, unsigned pixelSize, unsigned **pixels);

// test functions - erase later
// void testImgRead(char *inputFilename, char *outputFilename);
void testVQ(char *filenameInput, char *filenameOutput);