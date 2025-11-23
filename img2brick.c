#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


int width;
int height;
int catSize;



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




///////////////////////////////////Utils Functions//////////////////////////////////////

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




///////////////////////////////////FORMATTING FUNCTIONS//////////////////////////////////////

// Converts the string image into a proper ColorValues matrix
ColorValues* loadImage(const char* path) {

    // we open the file
    FILE* image = fopen(path, "r");
    if (!image) {
        perror("Error opening file");
        exit(1);
    }
    
    // this is the format of a "pixel" in the initial file
    char hex[7];

    // allocate memory dynamically, still assuming we were given the dimensions of the image.
    ColorValues* pixels = malloc((width * height) * sizeof(ColorValues));
    if (!pixels) {
        perror("Memory allocation failed");
        fclose(image);
        exit(1);
    }

    // a counter deemed useful to fill in our pixel array
    int count = 0;
    while (fscanf(image, "%6s", hex) == 1) {
        // convert hex string into a proper RGB
        pixels[count] = hexToRGB(hex);
        count++;
    }

    // Closing the file
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

        snprintf(catalog[i].name, sizeof(catalog[i].name), "%dx%d/%s", w, h, hex);
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
    getDims("image.txt", &width, &height);
    ColorValues* img = loadImage("image.txt");

    // making the tiling and putting it in a file
    toBrick_1x1(img, catalog, catSize);

    free(img);
}