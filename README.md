# PNG decoder library for low memory MCUs

Extract PNG files with a low memory footprint.

Can be used to "stream" image data to a larger display device. 

The library implements TinyDeflate from https://github.com/bisqwit/TinyDeflate

## (Current) Limitations

- The input (compressed) PNG image needs to fit into memory
- No support for palette based PNGs
- Requires C++17 

## Usage example

```
#include "tinyPNG.h"

tinyPNG png;

// ...

int y = 0;
try
{
    png.setPNG(input_buffer, input_size);
    printf("Width: %d\n", png.getWidth());
    printf("Height: %d\n", png.getHeight());
    printf("Components: %d\n", png.getComponents());

    png.process(
        [](unsigned char *line) 
        {
            int x;
            for (x = 0; x < png.getWidth(); x++)
            {
                // ... x, y ...
                // On RGB:
                // RED:   line[(x * 3) + 0]
                // GREEN: line[(x * 3) + 1]
                // BLUE:  line[(x * 3) + 2]
            }
            y++;
        }
    );
}    
catch (string s)
{
    printf("%s\n", s.c_str());
    return 0;
}
```

## More examples

https://github.com/olliiiver/tinyPNG_platformio_example
