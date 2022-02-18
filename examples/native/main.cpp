#if defined(NATIVE)

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#include "image.h"
#include "tinyPNG.h"

using namespace std;
using namespace cv;

tinyPNG png;

unsigned char *buf;
int pos = 0;

void convertGray(unsigned char *line)
{
    int x;
    if (png.getBytesPerPixel() > 1)
    {
        for (x = 0; x < png.getWidth(); x++)
        {
            line[x] = (line[x * png.getBytesPerPixel() + 1] > 0xAA) ? 0xFF : 0x00;
        }
    }
}

int main()
{
    try
    {
        png.setPNG(image, 40187);
        printf("Width: %d\n", png.getWidth());
        printf("Height: %d\n", png.getHeight());
        printf("Theoretical deflated size: %d\n", png.getDeflatedSize());

        buf = (unsigned char *)malloc(png.getBytesPerLine() * png.getHeight());
        png.process(
            [](unsigned char *line) {
                int x;
                for (x = 0; x < png.getBytesPerLine(); x++)
                {
                    buf[pos++] = line[x];
                }
            });
    }
    catch (string s)
    {
        printf("%s\n", s.c_str());
        return 0;
    }

    cv::Mat image = cv::Mat(png.getHeight(), png.getWidth(), (png.getBytesPerPixel() == 4) ? CV_8UC4 : CV_8UC3, buf);
    cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
    cv::namedWindow("Display window", WINDOW_AUTOSIZE);
    cv::imshow("Display window", image);
    waitKey(0);
}

#endif
