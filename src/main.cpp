

#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"
#include "image.h"
#include "processor.h"


template<typename T>
std::string toJson(const IImage<T>& image, const std::vector<PixelCoord>& pixels) 
{
    std::string json = "{ \"pixels\": [";
    for (size_t i = 0; i < pixels.size(); ++i) {
        int pixelValue = image.getPixelValue(pixels[i].x, pixels[i].y);
        json += "{";
        json += "\"x\": " + std::to_string(pixels[i].x) + ",";
        json += "\"y\": " + std::to_string(pixels[i].y) + ",";
        json += "\"value\": " + std::to_string(pixelValue);
        json += "}";
        if (i < pixels.size() - 1) json += ",";
    }
    json += "], \"number\": ";
    json += std::to_string(pixels.size()) + "} \n";
    return json;
}


int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <image_path> <top_n> <output_json_path>\n";
        return 1;
    }

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
    

        std::string jsonOutput = toJson(*image, topPixels);

        std::ofstream outputFile(outputJsonPath);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Could not open the output file.");
        }
        outputFile << jsonOutput;
        outputFile.close();

        std::cout << "JSON file has been successfully generated at: " << outputJsonPath << "\n";

        //Parallel
        topPixels = ip.processImageParallel(topN);

        jsonOutput = toJson(*image, topPixels);
        outputFile.open(outputJsonPathParallel);
        if (!outputFile.is_open()) {
            throw std::runtime_error("Could not open the output file.");
        }
        outputFile << jsonOutput;
        outputFile.close();


    } catch (const std::exception& e) {
        std::cerr << "Error occurred: " << e.what() << "\n";
        return 1;
    }

    return 0;
};


