
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
#include <opencv2/highgui/highgui.hpp>

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
            image.at<uchar>(i, j) = 17;
        }
    }

    return image;
}

cv::Mat generateMatrix(size_t size) {
    cv::Mat image(size, size, CV_16U);

    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            uint16_t value = static_cast<uint16_t>(rand() % 65536); // 2^16 = 65536
            image.at<uint16_t>(i, j) = value;
        }
    }

    //cv::imshow("Generated Image", image);
    //cv::waitKey(0);

    return image;
}

cv::Mat generateMatrixSort(size_t size, int increment) {
    uint16_t start = 0;
    cv::Mat image(size, size, CV_16U);

    if(increment < 0) start = size * size - 1;
    for (size_t i = 0; i < size; ++i) {
        for (size_t j = 0; j < size; ++j) {
            image.at<uint16_t>(j, i) = start;
            start += increment;
        }
    }

    //cv::imshow("Generated Image", image);
    //cv::waitKey(0);
    return image;
}

cv::Mat generateMatrixSortA(size_t size) {
    return generateMatrixSort(size, 1);
}
cv::Mat generateMatrixSortD(size_t size) {
    return generateMatrixSort(size, -1);
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
    double startPercentage = 0.001; // Start cu 0.1%
    double endPercentage = 0.999;   // End cu 99.9%
    double step = (endPercentage - startPercentage) / (steps - 1);

    for (size_t i = 0; i < steps; ++i) {
        double currentPercentage = startPercentage + step * i;
        topNValues.push_back(static_cast<size_t>(currentPercentage * totalPixels));
    }

    return topNValues;
}


void processAndMeasure(size_t idx, size_t dimm, size_t topN, 
    const std::string& outputFile, 
    ImageProcessor<uint16_t>& ip,
    size_t processid,
    std::vector<std::function<std::vector<PixelCoord>(ImageProcessor<uint16_t>&, size_t)>>& processFunctions

) {
    
    if (processid >= processFunctions.size()) {
            std::cout << "ID unavailable!." << std::endl;
            return;
    } 

    struct rusage usageStart, usageEnd;
    getrusage(RUSAGE_SELF, &usageStart);
    auto start = std::chrono::high_resolution_clock::now();
    auto topPixels = processFunctions[processid](ip, topN);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> execTime = end - start;
    getrusage(RUSAGE_SELF, &usageEnd);
    
    long memoryUsed = usageEnd.ru_maxrss - usageStart.ru_maxrss;
    long cost = execTime.count() * 0.8 + memoryUsed * 0.2;

    std::ofstream file(outputFile, std::ios_base::app);
    file <<  idx << "," << dimm << "," << topN << "," << processid << 
"," << execTime.count() << "," << memoryUsed <<  "," << cost << "\n";
    file.close();

    std::cout << "TopN: " << topN << ", IDMethod: " << processid << 
    ", ExecTime: " << execTime.count() << " ms, MemUsed: " << memoryUsed << " KB" << std::endl;
}

#define GENERATE_FUNC_PAIR(func) {func, #func}

int simulate(size_t maxImgColAndRowSize, size_t topNgranularity) {

  //init CSV with head
    std::string outputFile = "matrix_data.csv";
    std::ofstream file(outputFile);
    file << "Type,Dimension,topN,IDMethod,ExecTime,MemUsed,Cost\n";
    file.close();


    std::vector<std::function<std::vector<PixelCoord>(ImageProcessor<uint16_t>&, size_t)>>
    processFunctions = {
        &ImageProcessor<uint16_t>::processImageSet,
        &ImageProcessor<uint16_t>::processImageParallelV2,
        &ImageProcessor<uint16_t>::processImageHeapBest,
        &ImageProcessor<uint16_t>::processImageCS

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

    size_t idx = 0;
    for (auto& generatorWithDesc : matrixGenerators) {
        auto generateMatrix = generatorWithDesc.first; // fc generator
        const auto& description = generatorWithDesc.second; // Descrierea

        std::cout << std::endl;
        std::cout << description << std::endl ;
    
        for (size_t size = 100; size <= maxImgColAndRowSize; size *= 2) {
            cv::Mat image = generateMatrix(size);
            ImageWrapper<uint16_t> wrapper(image);
            ImageProcessor<uint16_t> ip(wrapper);
            //auto wrapper = createImageWrapper(image);
            //auto ip = createImageProcessor(std::move(wrapper));
            
             std::cout << "size " << size << std::endl ;
            //get relevant topN values
            std::vector<size_t> topNValues = generateTopNValues(size, topNgranularity); // Generate 5 values
            for (auto topN : topNValues) {
                //std::cout << "Img row: " << image.rows << " img col: " <<  image.cols << std::endl;    
                std::cout << "Running: " << description << " | Img row: " << image.rows << " img col: " << image.cols << std::endl;      
                for(size_t i = 0; i < processFunctions.size(); ++i) {                     
                    processAndMeasure(idx, wrapper.size(), topN,  outputFile, ip, i, processFunctions); 
                }
            }
        }

        ++idx;
    }


    return 0;
}






int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <image_path> <top_n> <output_json_path>\n";
        return 1;
    }
    std::string imagePath = argv[1];
    size_t topN = std::stoi(argv[2]);
    std::string outputJsonPath = argv[3];
    std::string outputJsonPathParallel = std::string("Parallel_") + argv[3];

    size_t topNgranularity = 10;
    size_t maxImgColAndRowSize = 1000; // Max image row and col size

    simulate(maxImgColAndRowSize, topNgranularity);

    try {
       

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


