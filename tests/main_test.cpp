#include <gtest/gtest.h>
#include <chrono>
#include <random>
#include <tuple>
#include <vector>

#include "utils.h"
#include "image.h"
#include "processor.h"
#include "utils.h"
#include "image.h"
#include "processor.h"

size_t topN = 100000;

//
// HELPERS FOR UNIT TESTS
//
//todo: move in dedicated location

template<typename T>
std::vector<std::tuple<T, int, int>>
 generatePixels(int numPositions, int minX, int maxX, int minY, int maxY, 
                int minColor, int maxColor) {
    if (numPositions > (maxX - minX + 1) * (maxY - minY + 1)) {
        std::cout << 
        ("Not enough unique positions available"
         "for the given interval and number of positions.")
         << std::endl;
    }

    std::vector<std::tuple<T, int, int>> positions;
    std::random_device rd;
    std::mt19937 gen(rd());

    for (int i = 0; i < numPositions; ++i) {
        int x, y, color;

        // Generate unique positions and colors within the range
        do {
            x = std::uniform_int_distribution<int>(minX, maxX)(gen);
            y = std::uniform_int_distribution<int>(minY, maxY)(gen);
            color = std::uniform_int_distribution<int>(minColor, maxColor)(gen);
        } while (
            std::find_if(positions.begin(), positions.end(), 
                [x, y](const auto& position) {
                   return std::get<1>(position) == x && std::get<2>(position) == y;
                }) != positions.end()
            );

        positions.emplace_back(color, x, y);
    }

    return positions;
}

template<typename T>
void sortPixelByValue(std::vector<PixelCoord>& pixels, const IImage<T>& img) {
    std::sort(pixels.begin(), pixels.end(), [&img](const PixelCoord& a, const PixelCoord& b) {
        return img.getPixelValue(a.x, a.y) > img.getPixelValue(b.x, b.y);
    });
}

template<typename T>
std::vector<PixelCoord> sortImageAndGetTopN(const IImage<T>& image, size_t topN) {
    std::vector<std::pair<PixelCoord, T>> pixels;

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T value = image.getPixelValue(x, y);
            pixels.emplace_back(PixelCoord{x, y}, value);
        }
    }

    // Sort pixels by value in descending order.
    std::sort(pixels.begin(), pixels.end(), [](const auto& a, const auto& b) {
        return a.second > b.second; 
    });

    //Build the result vector with the top N pixels (only their coordinates).
    std::vector<PixelCoord> result;
    for (size_t i = 0; i < std::min(topN, pixels.size()); ++i) {
        result.push_back(pixels[i].first);
    }

    return result;
}

template<typename T>
void printTopN(const IImage<T>& img, std::vector<PixelCoord> &topNpix) 
{
    std::cout << "Top N pixeli descrescator dupa valoarea culorii:\n";
    for (const auto& coord : topNpix) {
        auto [ x, y] = coord;
        auto color = img.getPixelValue(x, y);
        std::cout << "Color: " << color << " at (" << x << ", " << y << ")\n";
    }
}

bool comparePixelCoord(std::vector<PixelCoord> &v1, std::vector<PixelCoord> &v2) 
{    
    EXPECT_EQ(v1.size(), v2.size());
 
    for (size_t i = 0; i < v1.size(); ++i) {
        EXPECT_EQ(v1[i].x, v1[i].x);
        EXPECT_EQ(v2[i].y, v2[i].y);
        if((v1[i].x != v2[i].x) || (v1[i].y!= v2[i].y))
            return false;
    }

    return true;
}



//
// UNIT TESTS
//

// Test for basic functionality verification 
TEST(Test, TestFunctionality1) {
    unsigned int topN = 1;
    VectorImage<uint16_t> img({{255, 1, 1}, {777, 5, 6}, {150, 3, 3}});
    img.printImage();
    //auto sortedPixels = img.getPixelsSortedByValue();

    ImageProcessor<uint16_t> ip(img);
    std::vector<PixelCoord> topNpix = ip.processImage(topN);
    ASSERT_LE(topNpix.size() , topN);
    ASSERT_GT(topNpix.size() , 0);
    ASSERT_EQ(topNpix[0].x , 5);
    ASSERT_EQ(topNpix[0].y , 6);
     
    for (const auto& pixelCoord : topNpix) {
        std::cout << 
        "PixelCoord x: " << pixelCoord.x << ", y: " << pixelCoord.y << "\n";
    }

    VectorImage<uint16_t> img2({{888, 0, 0}, {777, 5, 6}, {150, 3, 3}});
    ImageProcessor<uint16_t> ip2(img2);
    topNpix = ip2.processImage(topN);
    img2.printImage();
    ASSERT_LE(topNpix.size() , topN);
    ASSERT_GT(topNpix.size() , 0);
    ASSERT_EQ(topNpix[0].x , 0);
    ASSERT_EQ(topNpix[0].y , 0);


    VectorImage<uint16_t> img3({{1, 0, 0}, {777, 2, 3}, {989, 6, 5}});
    ImageProcessor<uint16_t> ip3(img3);
    topNpix = ip3.processImage(topN);
    img3.printImage();
    ASSERT_LE(topNpix.size() , topN);
    ASSERT_GT(topNpix.size() , 0);
    ASSERT_EQ(topNpix[0].x , 6);
    ASSERT_EQ(topNpix[0].y , 5);

    VectorImage<uint16_t> img4({{1, 0, 0}, {777, 2, 3}, {777, 3, 5}});
    ImageProcessor<uint16_t> ip4(img4);
    topNpix = ip4.processImage(topN);
    img4.printImage();
    ASSERT_LE(topNpix.size() , topN);
    ASSERT_GT(topNpix.size() , 0);
    ASSERT_EQ(topNpix[0].x , 2);
    ASSERT_EQ(topNpix[0].y , 3);
    ASSERT_EQ(topNpix[1].x , 3);
    ASSERT_EQ(topNpix[1].y , 5);
}

TEST(Test, TestFunctionality2) {
    unsigned int topN = 2;
    VectorImage<uint16_t> img({{255, 1, 1}, {777, 5, 6}, {150, 3, 3}, {777, 4, 6}});
    img.printImage();
    //auto sortedPixels = img.getPixelsSortedByValue();

    ImageProcessor<uint16_t> ip(img);
    std::vector<PixelCoord> topNpix = ip.processImage(topN);
    ASSERT_EQ(topNpix.size() , topN);
    ASSERT_EQ(topNpix[0].x , 4);
    ASSERT_EQ(topNpix[0].y , 6);
    ASSERT_EQ(topNpix[1].x , 5);
    ASSERT_EQ(topNpix[1].y , 6);
     
    for (const auto& pixelCoord : topNpix) {
        std::cout << 
        "PixelCoord x: " << pixelCoord.x << ", y: " << pixelCoord.y << "\n";
    }
}

// Test for basic functionality verification 
TEST(Test, TestFunctionality3) {
    VectorImage<uint16_t> img({{255, 1, 1}, {100, 2, 2}, {150, 3, 3}});
    ImageProcessor<uint16_t> ip(img);

    for(size_t topN = 1; topN < 100; ++topN) {
        //std::cout <<"topN " << topN << std::endl;
        std::vector<PixelCoord> topNpix = ip.processImage(topN);
        ASSERT_LE(topNpix.size() , topN);
        if(topNpix.size() < topN) {
            ASSERT_EQ(img.size(), topNpix.size()) ;
        }

        sortPixelByValue(topNpix, img);
        std::vector<PixelCoord> pixImg =
        sortImageAndGetTopN<uint16_t>(img, topN);

        comparePixelCoord(topNpix, pixImg);
    }
}

// Boundary test (ex, empty image)
TEST(ImageProcessing, EdgeCases1) {
    unsigned int topN = 0;

    const std::vector<std::tuple<uint16_t, int, int>> vec;
    VectorImage<uint16_t> emptyImg({});
    emptyImg.printImage();
    auto sortedPixels = emptyImg.getPixelsSortedByValue();

    ImageProcessor<uint16_t> ip(emptyImg);
    std::vector<PixelCoord> topNpix = ip.processImage(topN);
    ASSERT_EQ(topNpix.size() , 0);
    
    topN = 1;
    topNpix = ip.processImage(topN);
    ASSERT_EQ(topNpix.size() , 0);
}

// Boundary test - all pixels black
TEST(ImageProcessing, EdgeCases2) {
   
    VectorImage<uint16_t> img({{0, 1, 1}, {0, 0, 0}, {0, 2, 2}});
    ImageProcessor<uint16_t> ip(img);

    for(size_t topN = 0; topN < 5; ++topN) {
        std::cout <<"topN " << topN << std::endl;
        std::vector<PixelCoord> topNpix = ip.processImage(topN);
        ASSERT_LE(topNpix.size() , topN);
        if(topNpix.size() < topN) {
            ASSERT_EQ(img.size(), topNpix.size()) ;
        }

        sortPixelByValue(topNpix, img);
        std::vector<PixelCoord> pixImg =
        sortImageAndGetTopN<uint16_t>(img, topN);

        comparePixelCoord(topNpix, pixImg);
    }
}


// Test for checking random functionality
TEST(Test, RandomImageGeneration) {
  
    VectorImage<uint16_t> img({{255, 1, 1}, {100, 2, 2}, {150, 3, 3}});
    //img.printImage();
    //auto sortedPixels = img.getPixelsSortedByValue();

    unsigned int maxTopN = 12;
    unsigned int maxPixels = 8;
    unsigned int minX = 0, maxX = 10, minY = 0, maxY = 10, minColor = 0, maxColor = 65535;

    for(unsigned int numPixels = 0; numPixels < maxPixels; ++numPixels) {
        std::cout << "Image generated with " << numPixels << " pixels" << std::endl;
        VectorImage<uint16_t> img(generatePixels<uint16_t>(
            numPixels, minX, maxX, minY, maxY, minColor, maxColor));
        ImageProcessor<uint16_t> ip(img);

        for(unsigned int topN = 0; topN < maxTopN; ++topN ) {
            std::cout << topN << " Pixels with the highest pixel values." << std::endl;
            std::vector<PixelCoord> topNpix = ip.processImage(topN);
            //printTopN(img, topNpix);
            ASSERT_LE(topNpix.size() , topN);
            if(topNpix.size() < topN) {
                ASSERT_EQ(img.size(), topNpix.size()) ;
            }

            sortPixelByValue(topNpix, img);
            std::vector<PixelCoord> pixImg =
            sortImageAndGetTopN<uint16_t>(img, topN);
            comparePixelCoord(topNpix, pixImg);
        }
    }
}


// Performance test
TEST(ImageProcessingPerformance, processImageHeap) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 200; ++i) {
        pixels.push_back(std::make_tuple(i*2, i, 3*i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageHeap(topN);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}


// Performance test
TEST(ImageProcessingPerformance, processImageHeapNice) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 200; ++i) {
        pixels.push_back(std::make_tuple(i*2, i, 3*i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageHeapUnrolling(topN);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}


// Performance test
TEST(ImageProcessingPerformance, processImageParallel) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 200; ++i) {
        pixels.push_back(std::make_tuple(i*2, i, 3*i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageParallel(topN);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}



// Performance test
TEST(ImageProcessingPerformance, processImageCS) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 200; ++i) {
        pixels.push_back(std::make_tuple(i*2, i, 3*i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageCS(topN);//processImageCS_MAP
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}


// Performance test
TEST(ImageProcessingPerformance, processImageCS_MAP) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 200; ++i) {
        pixels.push_back(std::make_tuple(i*2, i, 3*i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageCS_MAP(topN);//processImageCS_MAP
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}


// Performance test
TEST(ImageProcessingPerformance, processImageSet) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 200; ++i) {
        pixels.push_back(std::make_tuple(i/2, i*3, i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageSet(topN);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}


// Performance test
TEST(ImageProcessingPerformance, processImageSort) {
    std::vector<std::tuple<uint16_t, int, int>> pixels 
                = {{255, 1, 1}, {100, 2, 2}, {150, 3, 3}};

    for(int i = 0; i < 100; ++i) {
        pixels.push_back(std::make_tuple(i/2, i*3, i));
    };

    VectorImage img(pixels);

    ImageProcessor<uint16_t> processor(img);

    auto start = std::chrono::high_resolution_clock::now();
    auto topNpix = processor.processImageSort(topN);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";

    ASSERT_LE(topNpix.size(), topN);

}


//
//
//

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


