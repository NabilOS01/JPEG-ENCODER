# JPEG ENCODER
## Overview :
This is a JPEG encoder done as a school project using the C language. It encodes both ".ppm" and ".pgm" files, and outputs a JPEG image.
## Usage :
* First, the compilation is done by using the command : `make`. This will generate an executable named "ppm2jpeg".

* Second, you can run the executable using the command : `./ppm2jpeg input-image`. For example, the directory images contains some test images, to generate a JPEG image corresponding to thumbs.ppm, use the command : `./ppm2jpeg images/thumbs.ppm`.

* Additional options can be given, such as `--sample` which lets you choose the sampling factors, `--outfile` which lets you choose the name and location of the generated jpeg image, `--dct` which lets you choose the algorithm used to calculate the discrete cosine transform. To see how to use these options, run the command `./ppm2jpeg --help`.

* Please note that by setting `--dct=loeffler`, the jpeg image will be generated a lot faster, however when the sampling factors are too high, the quality might be lower. However, `--dct=naive` keeps a good quality, but takes more time to run. By default ( aka if you don't specify the `--dct` option ), the naive method will be used.