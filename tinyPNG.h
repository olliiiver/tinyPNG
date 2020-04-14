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

#ifndef TINYPNG_H
#define TINYPNG_H

class tinyPNG
{

private:
    int width;
    int height;
    int bitdepth;
    int colortype;
    int interlace;
    int components;
    int deflatedSize;

    unsigned char *obj;
    unsigned char *data;
    unsigned int size;

public:
    void setPNG(unsigned char *d, unsigned int s);
    void process(void (*callback)(unsigned char *line));
    int getWidth() { return width; }
    int getHeight() { return height; }
    int getDeflatedSize() { return deflatedSize; }
    int getBPP();
    int getBytesPerPixel();
    int getBytesPerLine();
    int getComponents();

private:
    void processIDAT(int pos, int size, void (*callback)(unsigned char *line));
    void unfilter(int filterType, unsigned char *lineCurr, unsigned char *linePrev);
    int paeth(int a, int b, int c);
    
};

#endif
