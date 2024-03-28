
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


cv::Mat generateMatrixGray(size_t size) {
    cv::Mat image(size, size, CV_8UC1);

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            uchar value = static_cast<uchar>(rand() % 256);
            image.at<uchar>(i, j) = value;
        }
    }

    return image;
}

cv::Mat generateMatrixUniform(size_t size) {
    cv::Mat image(size, size, CV_8UC1);

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            uchar value = static_cast<uchar>(rand() % 256);
            image.at<uchar>(i, j) = 17;
        }
    }

    return image;
}

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

cv::Mat generateMatrixSortA(size_t size) {
    return generateMatrixSort(size, 1);
}
cv::Mat generateMatrixSortD(size_t size) {
    return generateMatrixSort(size, -11);
}


cv::Mat generateMatrixWithDistribution(size_t size) {
    size_t regionsX = 4; // nb region per h.
    size_t regionsY = 4; // nb region per v.

    cv::Mat image = generateMatrixWithVariableRegions(size, regionsX, regionsY);

    //cv::imshow("Generated Image", image);
    //cv::waitKey(0);

    return image;
}

std::vector<size_t> generateTopNValues(size_t size, size_t steps) {
    size_t totalPixels = size * size;
    std::vector<size_t> topNValues;
    double startPercentage = 0.01; // Start cu 1%
    double endPercentage = 0.99;   // End cu 99%
    double step = (endPercentage - startPercentage) / (steps - 1);

    for (size_t i = 0; i < steps; ++i) {
        double currentPercentage = startPercentage + step * i;
        topNValues.push_back(static_cast<size_t>(currentPercentage * totalPixels));
    }

    return topNValues;
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

#define GENERATE_FUNC_PAIR(func) {func, #func}

int simulate() {

  //init CSV with head
    std::string outputFile = "matrix_data.csv";
    std::ofstream file(outputFile);
    file << "Dimension,topN,IDMethod,ExecTime,MemUsed,Cost\n";
    file.close();


    std::vector<std::function<std::vector<PixelCoord>(ImageProcessor<uint16_t>, int)>> processFunctions = {
        &ImageProcessor<uint16_t>::processImageParallel,
        &ImageProcessor<uint16_t>::processImageHeap
        // add here new fc for testing
    };

   // matrix list generators
    using MatrixGeneratorFunc = std::function<cv::Mat(size_t)>;
    using MatrixGeneratorWithDescription = std::pair<MatrixGeneratorFunc, std::string>;
    std::vector<MatrixGeneratorWithDescription> matrixGenerators = {
        {generateMatrix, "random Matrix"},
        {generateMatrixSortA, "Sorted Ascending"},
        {generateMatrixSortD, "Sorted Descending"},
        {generateMatrixWithDistribution, "With Specific Distribution"},
        {generateMatrixUniform, "Uniform Distribution"}
        // add here static fc for generate matrix with description
    };
    
    size_t presetMaxSize = 10000; // Max image row and col size

    for (auto& generatorWithDesc : matrixGenerators) {
        auto generateMatrix = generatorWithDesc.first; // Funcția generator
        const auto& description = generatorWithDesc.second; // Descrierea

        std::cout << std::endl;
        std::cout << description << std::endl ;
    
        for (size_t size = 500; size <= presetMaxSize; size *= 2) {
            cv::Mat image = generateMatrix(size);
            ImageWrapper wrapper(image);
            ImageProcessor<uint16_t> ip(wrapper);
            
             std::cout << "size " << size << std::endl ;
            //get relevant topN values
            std::vector<size_t> topNValues = generateTopNValues(size, 5); // Generate 5 values
            for (auto topN : topNValues) {
                //std::cout << "Img row: " << image.rows << " img col: " <<  image.cols << std::endl;    
                std::cout << "Running: " << description << " | Img row: " << image.rows << " img col: " << image.cols << std::endl;      
                for(size_t i = 0; i < processFunctions.size(); ++i) {                     
                    processAndMeasure(wrapper.size(), topN, i, outputFile, ip, processFunctions); 
                }
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
        size_t topN = std::stoi(argv[2]);
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


