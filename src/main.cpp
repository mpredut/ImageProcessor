
#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <chrono>

#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"
#include "image.h"
#include "processor.h"


// GenerateMatrix.cpp
#include <opencv2/opencv.hpp>
#include <cstdlib> // Pentru rand()


cv::Mat generateMatrix(size_t size) {
    cv::Mat image(size, size, CV_8UC3);

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            uchar blue = static_cast<uchar>(rand() % 256);
            uchar green = static_cast<uchar>(rand() % 256);
            uchar red = static_cast<uchar>(rand() % 256);
            image.at<cv::Vec3b>(i, j) = cv::Vec3b(blue, green, red);
        }
    }

    return image;
}

cv::Mat generateMatrixSort(size_t size, int increment) {
    int start = 0;
    cv::Mat image(size, size, CV_8UC3);

    if(increment < 0) start = size * size - 1;
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            image.at<cv::Vec3b>(i, j) = start;
            start =+ increment;
        }
    }

    return image;
}



void processAndMeasure(size_t dimm, size_t topN, size_t processImageId, 
    const std::string& outputFile, 
    ImageProcessor<uint16_t>& ip,
    std::vector<std::function<std::vector<PixelCoord>(ImageProcessor<uint16_t>, int)>>& processFunctions
) {
    
    if (processImageId >= processFunctions.size()) {
            std::cout << "ID unavailable!." << std::endl;
            return;
    } 

    struct rusage usageStart, usageEnd;
    getrusage(RUSAGE_SELF, &usageStart);
    auto start = std::chrono::high_resolution_clock::now();

    auto topPixels = processFunctions[processImageId](ip, topN);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> execTime = end - start;
    getrusage(RUSAGE_SELF, &usageEnd);
    
    long memoryUsed = usageEnd.ru_maxrss - usageStart.ru_maxrss;
    long cost = execTime.count() * 0.7 + memoryUsed * 0.3;
    std::ofstream file(outputFile, std::ios_base::app);
    file << dimm << "," << topN << "," << processImageId << 
    "," << execTime.count() << "," << memoryUsed <<  "," << cost << "\n";
    file.close();

    std::cout << "TopN: " << topN << ", IDMethod: " << processImageId << 
    ", ExecTime: " << execTime.count() << " ms, MemUsed: " << memoryUsed << " KB" << std::endl;
}


int simulate() {

    std::vector<std::function<std::vector<PixelCoord>(ImageProcessor<uint16_t>, int)>> processFunctions = {
        &ImageProcessor<uint16_t>::processImage,
        &ImageProcessor<uint16_t>::processImage
        // Adaugă aici alte metode dacă există
    };

    size_t presetSize = 2000; // Max image size
    std::string outputFile = "matrix_data.csv";

    //init CSV with head
    std::ofstream file(outputFile);
    file << "Dimension,topN,IDMethod,ExecTime,MemUsed,Cost\n";
    file.close();

    for (size_t size = 50; size <= presetSize; size *= 2) {
        cv::Mat image = generateMatrix(size);
        ImageWrapper wrapper(image);
        ImageProcessor<uint16_t> ip(wrapper);
        
        for (size_t topN = 10; topN <= size * size ; topN *= 100) {
            std::cout << "Img row: " << image.rows << " img col: " <<  image.cols << std::endl;     
            for(size_t i = 0; i < processFunctions.size(); ++i) {                     
                processAndMeasure(wrapper.size(), topN, i, outputFile, ip, processFunctions); 
            }
        }
    }

    std::cout << "Ascending matrix" << std::endl;
    for (size_t size = 50; size <= presetSize; size *= 2) {
        cv::Mat image = generateMatrixSort(size, 1);
        ImageWrapper wrapper(image);
        ImageProcessor<uint16_t> ip(wrapper);
        
        for (size_t topN = 10; topN <= size * size ; topN *= 100) {
            std::cout << "Img row: " << image.rows << " img col: " <<  image.cols << std::endl;     
            for(size_t i = 0; i < processFunctions.size(); ++i) {                     
                processAndMeasure(wrapper.size(), topN, i, outputFile, ip, processFunctions); 
            }
        }
    }

      std::cout << "Descending matrix" << std::endl;
    for (size_t size = 50; size <= presetSize; size *= 2) {
        cv::Mat image = generateMatrixSort(size, -1);
        ImageWrapper wrapper(image);
        ImageProcessor<uint16_t> ip(wrapper);
        
        for (size_t topN = 10; topN <= size * size ; topN *= 100) {
            std::cout << "Img row: " << image.rows << " img col: " <<  image.cols << std::endl;     
            for(size_t i = 0; i < processFunctions.size(); ++i) {                     
                processAndMeasure(wrapper.size(), topN, i, outputFile, ip, processFunctions); 
            }
        }
    }


    return 0;
}






int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <image_path> <top_n> <output_json_path>\n";
        return 1;
    }

    simulate();

    try {
        std::string imagePath = argv[1];
        int topN = std::stoi(argv[2]);
        std::string outputJsonPath = argv[3];
        std::string outputJsonPathParallel = std::string("Parallel_") + argv[3];

        auto imageReader = ImageReaderFactory::createImageReader();
        auto image = imageReader->readImage(imagePath);

    auto start = std::chrono::high_resolution_clock::now();
        ImageProcessor<uint16_t> ip(*image.get());
        auto topPixels = ip.processImage(topN);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time to execute " << topN << " pixels: " << duration.count() << " ms\n";
    

        //std::string jsonOutput = toJson(*image, topPixels);

        std::ofstream outputFile(outputJsonPath);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Could not open the output file.");
        }
        //outputFile << jsonOutput;
        outputFile.close();

        std::cout << "JSON file has been successfully generated at: " << outputJsonPath << "\n";

        //Parallel
        topPixels = ip.processImageParallel(topN);

        //jsonOutput = toJson(*image, topPixels);
        outputFile.open(outputJsonPathParallel);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Could not open the output file.");
        }
        //outputFile << jsonOutput;
        outputFile.close();


    } catch (const std::exception& e) {
        std::cerr << "Error occurred: " << e.what() << "\n";
        return 1;
    }

    return 0;
};


