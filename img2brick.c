#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define NULL_PIXEL ((ColorValues){.r = -1, .g = -1, .b = -1})
int width;
int height;
int catSize;
int canvasDims; //canvas size for the padding


///////////////////////////////////STRUCTURES//////////////////////////////////////

// Making a struct to make color comparison easier
typedef struct {
    int r;
    int g;
    int b;
} ColorValues;

// Making a brick struct that uses the ColorValues struct
typedef struct {
    char name[32];
    int width;
    int height;
    int holes[16];
    ColorValues color;
    float price;
    int number;
} Brick;

// this will be useful to make the tiling as well as calculating its quality
typedef struct {
    int index;
    int diff;
} BestMatch;

// crucial for the quadtree, its a chained list element in its fundamentals, but it carries additional infos.
typedef struct Node {
    int x; 
    int y;
    int w;
    int h;
    int is_leaf;
    ColorValues avg;
    struct Node* child[4];
} Node;



///////////////////////////////////UTILS FUNCTIONS//////////////////////////////////////

// a very needed function that returns the diemsions of the image through pointers
void getDims(const char* path, int* outWidth, int* outHeight, int*outCanvasDims) {
    FILE* image = fopen(path, "r");
    if (!image) {
        perror("Error opening file");
        exit(1);
    }

    int width = 0;
    int height = 0;
    int canvasDims = 0;
    char hex[7]; // part of the pixel "pattern"
    int firstLineDone = 0; // its used so that we only need to calculate the width once
    int c;

    while (!firstLineDone && fscanf(image, "%6s", hex) == 1) {
        width++;
        c = fgetc(image);
        if (c == '\n' || c == EOF) {
            firstLineDone = 1;
        }
    }

    rewind(image);

    while ((c = fgetc(image)) != EOF) {
        if (c == '\n') {
            height++;
        }
    }

    // in the case where the image dims are n * 1
    if (width > 0 && height == 0) {
        height = 1;
    }

    fclose(image);
    *outWidth = width;
    *outHeight = height;
    *outCanvasDims = biggestPow2(MAX(width, height));
}

//turns a hexadecimal value into rgb
ColorValues hexToRGB(const char *hex) {
    ColorValues p;
    sscanf(hex, "%02x%02x%02x", &p.r, &p.g, &p.b);
    return p;
}

//this is useful to organize the holes into each bricks
void parseHoles(const char *str, int *holes) {
    if (strcmp(str, "-1") == 0) {
        for (int i = 0; i < 16; i++) holes[i] = -1;
        return;
    }
    int len = strlen(str);
    int k = 0;
    for (int i = 0; i < len && k < 16; i++) {
        holes[k++] = str[i] - '0';
    }
    for (; k < 16; k++) holes[k] = -1;
}

// A function to compare the color diff between a between two entities with ColorValues.
int compareColors(ColorValues p1, ColorValues p2) {
    return
           (p1.r - p2.r) * (p1.r - p2.r)
         + (p1.g - p2.g) * (p1.g - p2.g)
         + (p1.b - p2.b) * (p1.b - p2.b);
}

//a function that returns the closest power of 2 bigger than the argument
int biggestPow2(int n) {
    int p = 0;
    while (pow(2,p) < n) {
        p++;
    }
    return pow(2,p);
}

ColorValues averageColor(ColorValues* pixels, int regX, int regY, int w, int h){

    ColorValues average = {0, 0, 0};
    int count = 0;

    for(int y = regY; y < h; y++){
        for(int x = regX; x < w; x++){

            int current = y * w + x;
            average.r += pixels[current].r;
            average.g += pixels[current].g;
            average.b += pixels[current].b;
            count++;
        }
    }
    average.r /= count;
    average.g /= count;
    average.b /= count;

    return average;
}

Node* new_node(int x, int y, int w, int h, int is_leaf, ColorValues avg) {
    Node* n = malloc(sizeof(Node));
    n->x = x;
    n->y = y;
    n->w = w;
    n->h = h;
    n->is_leaf = is_leaf;
    n->avg = avg;

    for (int i = 0; i < 4; i++)
        n->child[i] = NULL;

    return n;
}

int doWeSplit(ColorValues* pixels, int x, int y, int w, int h, ColorValues avg, int thresh) {
    // maybe use the average color func instead
}

// a function that only serves the purpose of showing the canvas containing the image
void showCanvas(ColorValues* pixels) {
    for (int y = 0; y < canvasDims; y++) {
        for (int x = 0; x < canvasDims; x++) {
            ColorValues c = pixels[y * canvasDims + x];

            if (c.r < 0) {
                printf("NULL    ");
            } else {
                printf("#%02X%02X%02X ", c.r, c.g, c.b);
            }
        }
        printf("\n");
    }
}


///////////////////////////////////FORMATTING FUNCTIONS//////////////////////////////////////

// Converts the string image into a proper ColorValues matrix
ColorValues* loadImage(const char* path) {

    char hex[7]; // this is the format of a "pixel" in the initial file

    // we open the file
    FILE* image = fopen(path, "r");
    if (!image) {
        perror("Error opening file");
        exit(1);
    }

    //we setup the containers of our image
    ColorValues* temp = malloc(width * height * sizeof(ColorValues));
    ColorValues* pixels = calloc((canvasDims * canvasDims), sizeof(ColorValues));
    if (!temp || !pixels) {
        perror("Memory allocation failed");
        fclose(image);
        exit(1);
    }

    // we first transfer the image into temp
    int i = 0;
    while (fscanf(image, "%6s", hex) == 1 && i < width * height) {
        temp[i++] = hexToRGB(hex);
    }

    // we fill the canvas with nuill pixels to prepare it for the quadtree
    for (int i = 0; i < canvasDims * canvasDims; i++){
        pixels[i] = NULL_PIXEL;
    }

    // we transfer temp into the canvas
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y * canvasDims + x] = temp[y * width + x];
        }
    }

    showCanvas(pixels);  //if you wish to visualize the result.

    // Closing the file and freeing the memory
    free(temp);
    fclose(image);
    return pixels;
}

// loads in the catalog
Brick* loadCatalog(const char *path, int *outSize) {

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("Error opening catalog file");
        exit(1);
    }

    // On lit le nombre de lignes (donc nombre de briques)
    int n;
    if (fscanf(f, "%d\n", &n) != 1) {
        fprintf(stderr, "Invalid catalog file: missing line count\n");
        exit(1);
    }

    Brick *catalog = malloc(n * sizeof(Brick));
    if (!catalog) {
        perror("Memory allocation failed");
        exit(1);
    }

    for (int i = 0; i < n; i++) {
        int w, h, stock;
        char holesStr[16];
        char hex[16];
        float price;

        if (fscanf(f, " %d , %d , %15[^,] , %15[^,] , %f , %d",
                   &w, &h, holesStr, hex, &price, &stock) != 6)
        {
            fprintf(stderr, "Invalid line %d in catalog\n", i + 1);
            exit(1);
        }

        catalog[i].width  = w;
        catalog[i].height = h;
        catalog[i].price  = price;
        catalog[i].number = stock;
        catalog[i].color  = hexToRGB(hex);
        parseHoles(holesStr, catalog[i].holes);

        snprintf(catalog[i].name, sizeof(catalog[i].name), "%d-%d/%s", w, h, hex);
    }

    fclose(f);
    *outSize = n;
    return catalog;
}



///////////////////////////////////TILING ALGORYTHMS//////////////////////////////////////

// compares the color of the pixel to the one of every brick and returns the index of the best match
BestMatch findBestBrick(ColorValues pixel, Brick* catalog, int catSize) {

    //the necessary infos to get the optimal match from the brick catalog
    int minDiff = compareColors(pixel, catalog[0].color);
    int minIndex = 0;

    for (int i = 1; i < catSize; i++) {
        int diff = compareColors(pixel, catalog[i].color);
        if (diff < minDiff) {
            minDiff = diff;
            minIndex = i;
        }
    }
    BestMatch bestBrick = {minIndex, minDiff};
    return bestBrick;
}


void toBrick_1x1(ColorValues* pixels, Brick* catalog, int catSize) {

    // creating the file in which the tiling will be written
    FILE* tiled = fopen("tiled_image.txt", "w");
    if (!tiled) {
        perror("Error opening file");
    }

    // final price and total difference (used to calculate the quality) are stored here
    float totalPrice = 0;
    double totalDiff = 0;


    // double for loop that writes in the newly created file, mapping to each pixel the lego brick which's color is closest to its values
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            BestMatch bestBrick = findBestBrick(pixels[i * width + j], catalog, catSize);
            int bestId = bestBrick.index;
            totalPrice += catalog[bestId].price;
            totalDiff += bestBrick.diff;
            catalog[bestId].number--;
            fprintf(tiled, "%s %d %d\n", catalog[bestId].name, j, i);
        }
    }
    // Close the file
    fclose(tiled);

    double maxDiff = (255.0 * 255.0 * 3.0) * (width * height); // this is the maximum difference that is possible for a given image
    double quality = (1.0 - (totalDiff / maxDiff)) * 100.0; // the final quality of the tiler given in percentage


    printf("Tiled image saved to: %s, the lego recreation is %lf%% accurate, the price is %.2f€.\n", "tiled_image.txt", quality, totalPrice);

    //passing through the catalog to check if any bricks are missing from the stock
    int missingTotal = 0;
    for (int i = 0; i < catSize; i++) {
        if (catalog[i].number < 0) {
            int needed = -catalog[i].number;
            printf("%s:%d needed ", catalog[i].name, needed);
            missingTotal += needed;
        }
    }
    // if no bricks are missing
    if (missingTotal == 0) {
        printf(" STOCK IS OK");
    }
    printf("\n");

}

Node* quadTree(ColorValues* pixels, int x, int y, int w, int h, int thresh) {

    ColorValues avg = averageColor(pixels, x, y, w, h);

    if (doWeSplit(pixels, x, y, w, h, avg, thresh)) {

        int half_w = w / 2;
        int half_h = h / 2;

        Node* node = new_node(x, y, w, h, 0, avg);

        node->child[0] = quadTree(pixels, x,        y, half_w, half_h, thresh);
        node->child[1] = quadTree(pixels, x+half_w, y, half_w, half_h, thresh);
        node->child[2] = quadTree(pixels, x,        y+half_h, half_w, half_h, thresh);
        node->child[3] = quadTree(pixels, x+half_w, y+half_h, half_w, half_h, thresh);

        return node;
    }

    return new_node(x, y, w, h, 1, avg);
}


/////////////////////// MAIN FUNCTION //////////////////////////

int main() {

    // image.txt is in the format :

    //   FFFFFF FFFFFF FFFFFF\n
    //   FFFFFF FFFFFF FFFFFF\n
    //   FFFFFF FFFFFF FFFFFF\n

    // catalog.txt is in the format :

    //   6 (number of lines)
    //   1, 1, -1, ff0000, 0.10, 100
    //   1, 1, -1, 00ff00, 0.10, 100
    //   1, 1, -1, 0000ff, 0.10, 100
    //   1, 1, -1, ffff00, 0.10, 100
    //   1, 1, -1, ffffff, 0.10, 100
    //   1, 1, -1, 000000, 0.10, 100



    // prepare the inputs
    Brick *catalog = loadCatalog("catalog.txt", &catSize);
    getDims("image.txt", &width, &height, &canvasDims);
    ColorValues* img = loadImage("image.txt");

    // making the tiling and putting it in a file
    toBrick_1x1(img, catalog, catSize);

    free(img);
}