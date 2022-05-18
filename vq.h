#define MAX_WIDTH 2048
#define MAX_HEIGHT 2048

typedef struct img Img;
typedef struct block Block;
typedef struct blockMatrix BlockMatrix;

unsigned **getPixels(Img *img);
unsigned getWidth(Img *img);
unsigned getHeight(Img *img);
Img *readPgmImg(char *filename);

// test functions - erase later
// void testImgRead(char *inputFilename, char *outputFilename);
void testVQ(char *filename);