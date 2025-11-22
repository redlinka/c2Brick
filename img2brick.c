#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int width;
int height;

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
    int holes;
    ColorValues color;
    float price;
    int number;
} Brick;

// this will be useful to make the tiling as well as calculating its quality
typedef struct {
    int index;
    int diff;
} BestMatch;

// a very needed function that returns the diemsions of the image through pointers
void getDims(const char* path, int* outWidth, int* outHeight) {
    FILE* image = fopen(path, "r");
    if (!image) {
        perror("Error opening file");
        exit(1);
    }

    int width = 0;
    int height = 0;
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
}

// A function to compare the color diff between a between two entities with ColorValues.
int compareColors(ColorValues p1, ColorValues p2) {
    return
           (p1.r - p2.r) * (p1.r - p2.r)
         + (p1.g - p2.g) * (p1.g - p2.g)
         + (p1.b - p2.b) * (p1.b - p2.b);
}

// Converts the string image into a proper ColorValues matrix
ColorValues* pixelify(const char* path) {

    // we open the file
    FILE* image = fopen(path, "r");
    if (!image) {
        perror("Error opening file");
        return NULL;
    }
    
    // this is the format of a "pixel" in the initial file
    char hex[7];

    // allocate memory dynamically, still assuming we were given the dimensions of the image.
    ColorValues* pixels = malloc((width * height) * sizeof(ColorValues));
    if (!pixels) {
        perror("Memory allocation failed");
        fclose(image);
        return NULL;
    }

    // a counter deemed useful to fill in our pixel array
    int count = 0;
    while (fscanf(image, "%6s", hex) == 1) {
        ColorValues p;

        // convert hex string into a proper RGB
        sscanf(hex, "%02x%02x%02x", &p.r, &p.g, &p.b);
        pixels[count] = p;
        count++;
    }

    // Closing the file
    fclose(image);
    return pixels;
}

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
            fprintf(tiled, "%s", catalog[bestId].name);

            // only putting spaces between the bricks
            if (j < width - 1) {
                fprintf(tiled, " ");
            }
        }
        //end of a row
        fprintf(tiled, "\n");
    }
    // Close the file
    fclose(tiled);

    double maxDiff = (255.0 * 255.0 * 3.0) * (width * height); // this is the maximum difference that is possible for a given image
    double quality = (1.0 - (totalDiff / maxDiff)) * 100.0; // the final quality of the tiler given in percentage


    printf("Tiled image saved to: %s, the lego recreation is %lf%% accurate, the price is %.2f€.", "tiled_image.txt", quality, totalPrice);

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

/////////////////////// MAIN FUNCTION //////////////////////////

int main() {

    // image.txt is in the format :

    //   FFFFFF FFFFFF FFFFFF\n
    //   FFFFFF FFFFFF FFFFFF\n
    //   FFFFFF FFFFFF FFFFFF\n

    //  Waiting for feedback about this format.

    // an example catalog of bricks
    Brick catalog[] = {
        { "2x1 Red",     2, 1, 1, {255,   0,   0}, 0.10f, 100 },
        { "2x1 Green",   2, 1, 1, {  0, 255,   0}, 0.10f, 100 },
        { "2x1 Blue",    2, 1, 1, {  0,   0, 255}, 0.10f, 100 },
        { "2x1 Yellow",  2, 1, 1, {255, 255,   0}, 0.10f, 100 },
        { "2x1 White",   2, 1, 1, {255, 255, 255}, 0.10f, 100 },
        { "2x1 Black",   2, 1, 1, {  0,   0,   0}, 0.10f, 100 },
        { "2x1 Red",     1, 1, 1, {255,   0,   0}, 0.10f, 100 },
        { "2x1 Green",   1, 1, 1, {  0, 255,   0}, 0.10f, 100 },
        { "2x1 Blue",    1, 1, 1, {  0,   0, 255}, 0.10f, 100 },
        { "2x1 Yellow",  1, 1, 1, {255, 255,   0}, 0.10f, 100 },
        { "2x1 White",   1, 1, 1, {255, 255, 255}, 0.10f, 100 },
        { "2x1 Black",   1, 1, 1, {  0,   0,   0}, 0.10f, 100 },
    };
    // calcultaing its length for later use
    int catSize = sizeof(catalog) / sizeof(catalog[0]);

    getDims("image.txt", &width, &height);

    // turning the raw txt image file into an easily usable array
    ColorValues* img = pixelify("image.txt");

    free(img);
}