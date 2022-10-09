#ifndef PPM2JPEG_H
#define PPM2JPEG_H

#include <stdint.h>

struct arg
{
    uint8_t h1, h2, h3, v1, v2, v3;
    char *outfile;
    char *dct;
};

#endif /* PPM2JPEG_H */