/*

MIT License

Copyright (c) 2020 Oliver Kirst

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdio.h>
#include <iostream>
#include <cstring>

#include "gunzip.hh"
#include "tinyPNG.h"

using namespace std;

typedef enum color
{
    COLOR_LUM = 0,
    COLOR_RGB = 2,
    COLOR_INDEXED = 3,
    COLOR_LUMA = 4,
    COLOR_RGBA = 6
} color;

enum filter
{
    FILTER_NONE = 0,
    FILTER_SUB = 1,
    FILTER_UP = 2,
    FILTER_AVERAGE = 3,
    FILTER_PAETH = 4
} filter;

// Read 32-bit big-endian integer
auto I32 = [](unsigned char *d) {
    auto b = (unsigned char *)d;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | (b[3] << 0);
};

//  Constructor
void tinyPNG::setPNG(unsigned char *d, unsigned int s)
{
    data = d;
    size = s;
    process(NULL); // just load header
}

int tinyPNG::getComponents()
{
    return components;
}

int tinyPNG::getBPP()
{
    return bitdepth * components;
}

int tinyPNG::getBytesPerPixel()
{
    return bitdepth * components / 8;
}

int tinyPNG::getBytesPerLine()
{
    return getBytesPerPixel() * width;
}

void tinyPNG::processIDAT(int pos, int size, void (*callback)(unsigned char *line))
{
    unsigned x = ~0u, y;
    unsigned char ftype;
    unsigned char *lineCurr = (unsigned char *)malloc(getBytesPerLine()+1);
    unsigned char *linePrev = (unsigned char *)malloc(getBytesPerLine()+1);
    memset(linePrev, 0, getBytesPerLine());

    if (lineCurr == NULL || linePrev == NULL)
    {
        throw string("Can't allocate memory.");
    }

    Deflate(
        data + pos,
        size,
        [&](unsigned char byte) // This functor processes each uncompressed IDAT byte.
        {
            switch (x)
            {
            case ~0u: // "x" doubles as a coroutine status/goto flag.
                for (y = 0; y < height; ++y)
                {
                    ftype = byte;
                    x = ~1u;
                    return;
                case ~1u: // The first byte on each line is the filter type.
                    for (x = 0; x < getBytesPerLine(); ++x)
                    {
                        lineCurr[x] = byte;
                        if (x == getBytesPerLine() - 1)
                        {
                            /* end of line */
                            if (callback != NULL)
                            {
                                unfilter(ftype, lineCurr, linePrev);
                                callback(lineCurr);
                            }
                            memcpy(linePrev, lineCurr, width * getBPP() / 8);
                        }
                        return;
                    default:; // Read next byte
                    }
                }
            }
        });

    free(lineCurr);
    free(linePrev);
}

void tinyPNG::process(void (*callback)(unsigned char *line))
{
    unsigned pos, length;
    char chunk[5];

    /* check size */
    if (size < 29)
    {
        throw string("Invalid input. Too small.");
    }

    /* check PNG header */
    if (data[0] != 137 || data[1] != 80 || data[2] != 78 || data[3] != 71 || data[4] != 13 || data[5] != 10 || data[6] != 26 || data[7] != 10)
    {
        throw string("Input is not a PNG.");
    }

    for (pos = 8; pos < size; pos++)
    {
        length = I32(data + pos);
        pos += 4;
        chunk[0] = data[pos];
        chunk[1] = data[pos + 1];
        chunk[2] = data[pos + 2];
        chunk[3] = data[pos + 3];
        pos += 4;
        // printf("Chunk: %s at pos %d\n", chunk, pos);
        if (strncmp(chunk, "IHDR", 4) == 0)
        {
            width = I32(data + pos);
            height = I32(data + pos + 4);
            bitdepth = (int)data[pos + 8];
            colortype = (int)data[pos + 9];
            interlace = (int)data[pos + 10];
            switch (colortype)
            {
            case COLOR_LUM:
                components = 1;
                break;
            case COLOR_RGB:
                components = 3;
                break;
            case COLOR_LUMA:
                components = 2;
                break;
            case COLOR_RGBA:
                components = 4;
                break;
            default:
                throw string("Unsupported color type.");
            }
        }
        else if (strncmp(chunk, "IDAT", 4) == 0)
        {
            if (callback != NULL)
            {
                tinyPNG::processIDAT(pos, length, *callback);
            }
        }
        else
        {
            // printf("unknown header: %s\n", chunk);
        }
        pos += length + 3; // skip checksum
        deflatedSize = getBytesPerPixel() * getWidth() * getHeight();

    }
}

void tinyPNG::unfilter(int filterType, unsigned char *lineCurr, unsigned char *linePrev)
{
    int i;
    int bpp = getBPP() / 8;
    int stride = (width * bpp);

    switch (filterType)
    {
    case FILTER_NONE:
        break;
    case FILTER_SUB:
        for (i = bpp; i < stride; ++i)
            lineCurr[i] = lineCurr[i] + lineCurr[i - bpp];
        break;
    case FILTER_UP:
        for (i = 0; i < stride; ++i)
            lineCurr[i] = lineCurr[i] + linePrev[i];
        break;
    case FILTER_AVERAGE:
        for (i = 0; i < stride; ++i)
        {
            int last = i - bpp;
            int a = last >= 0 ? lineCurr[last] : 0;
            int b = linePrev[i];
            int avg = (a + b) / 2;
            lineCurr[i] = lineCurr[i] + avg;
        }
        break;
    case FILTER_PAETH:
        for (i = 0; i < stride; ++i)
        {
            int last = i - bpp;
            int a = last >= 0 ? lineCurr[last] : 0;
            int b = linePrev[i];
            int c = last >= 0 ? linePrev[last] : 0;
            lineCurr[i] = lineCurr[i] + paeth(a, b, c);
        }
        break;
    default:
        throw string("Unknown filter.");
    }
}

int tinyPNG::paeth(int a, int b, int c)
{
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else
        return c;
}
