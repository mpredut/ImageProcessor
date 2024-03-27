Description Project

ImageProcessor is a command-line application developed in C++ that processes an image
to find the top N brightest pixels and outputs the result as a JSON file. 
The design of application is using basic image processing load and JSON output generation using C++,OpenCV, and Google Test for unit testing.

Dependencies
C++11 compiler.
OpenCV (4.x) : Used for image loading
Google Test : Used for testing

Usage
To use the ImageProcessor application, the command syntax is as follows:
ImageProcessor <image_path> <bit_depth> <output_json_path>

<image_path>: The path to the input image file.
<bit_depth>: The bit depth of the image (e.g., 8 or 16).
<output_json_path> : The output path for the result JSON file.

Example:
./bin/ImageProcessor ./tests/new.png 50 out.json

Installation Instructions
1. Install dependencies
2. Configure makefile project.
3. Compile the Project: Use the provided Makefile to compile the project. 
4. run ImageProcessor bin file from ./bin directory.


1.Install dependencies - guide.
OpenCV:
Install OpenCV on your system using the package manager with the following 
"sudo apt-get install libopencv-dev"
Ensure that the paths to OpenCV are correctly set in your build configuration.
Google Test:
"
git clone https://github.com/google/googletest.git
cd googletest;mkdir build;cd build;cmake ..;make;sudo make install
"

2.Configuring the Makefile project
Before compiling the project, 
you need to configure OPENCV_INSTALL_DIR in the Makefile to match the installation directory of OpenCV on your system.
LDFLAGS and CXXFLAGS: Ensure these flags in your Makefile are configured to include the paths for OpenCV libraries and headers. 


3.Build project
make          # build the project
make test     # build the test application(s)
make run      # run all test application(s)
make clean    # cleanup binaries and intermediate file


Troubleshooting
If you encounter issues during the compilation or execution, check the following:
Verify the LDFLAGS and CXXFLAGS in the Makefile for correct paths to OpenCV libraries and headers.
Use pkg-config commands to ensure your system's configuration matches the expected library paths for OpenCV.
example : run "pkg-config --cflags opencv4" and "pkg-config --libs opencv4" and check the output against your LDFLAGS and CXXFLAGS configuration.




 
