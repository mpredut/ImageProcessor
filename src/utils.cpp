#include <string>

#include <iostream>
#include <fstream>
#include <vector>
#include<unordered_set>

#include <thread>
#include <mutex>
#include <algorithm>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>

#include "image.h"
#include "utils.h"

bool compareVectors(const std::vector<PixelCoord>& vec1, const std::vector<PixelCoord>& vec2) {
    // Check if the sizes of the vectors are different
    if (vec1.size() != vec2.size()) {
         std::cout <<"size missmatch "  << std::endl;
         return false;
    }

    // Compare each element of the vectors
    for (size_t i = 0; i < vec1.size(); ++i) {
        if (vec1[i].x != vec2[i].x) {
           std::cout <<"value mismatch x " <<vec2[i].x << " " <<  vec1[i].x << std::endl;
           return false;
        }
         if (vec1[i].y != vec2[i].y) {
           std::cout <<"value missmatch y " <<vec2[i].y << " " <<  vec1[i].y << std::endl;
           return false;
        }
    }
    return true;
}

template<typename T>
std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelVal<T>>
	mergeTopPixelQueues(const std::vector<std::priority_queue<PixelCoord, std::vector<PixelCoord>, 
                        ComparePixelVal<T>>>& pixelQueues, int topN) {
        std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelVal<T>> finalQueue;

        for (const auto& queue : pixelQueues) {
            std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelVal<T>> tempQueue = queue;
            while (!tempQueue.empty()) {
				int a = 1;
                finalQueue.push(tempQueue.top());
                tempQueue.pop();

                if (finalQueue.size() > topN) {
              	    finalQueue.pop();
                }
            };
        }

        return finalQueue;
};

template<typename T>
std::vector<PixelCoord> mergeLocalHeapsUsingPriorityQueue(const IImage<T>& img,
        const std::vector<std::vector<PixelCoord>>& localHeaps, size_t topN) {
    ComparePixelVal<T> comp(img);
    // Folosim un min-heap pentru că vrem să păstrăm cele mai mari valori (deci comparăm invers)
    auto compPriorityQueue = [&comp](const PixelCoord& a, const PixelCoord& b) { return comp(b, a); };
    std::priority_queue<PixelCoord, std::vector<PixelCoord>, decltype(compPriorityQueue)> pq(compPriorityQueue);

    // Încărcăm toate elementele în priority_queue
    for (const auto& heap : localHeaps) {
        for (const auto& item : heap) {
            pq.push(item);
            // Dacă depășim dimensiunea dorită, eliminăm elementul cu prioritatea cea mai mică
            if (pq.size() > topN) {
                pq.pop();
            }
        }
    }

    // Extragem elementele din priority_queue într-un vector, inversând ordinea
    // pentru că vrem ca elementul cu cea mai mare prioritate să fie la sfârșit
    std::vector<PixelCoord> result;
    while (!pq.empty()) {
        result.push_back(pq.top());
        pq.pop();
    }
    std::reverse(result.begin(), result.end());

    return result;
}  

//todo: remove that if not more needs
/*
static cv::Mat convertPixelsToMat(const std::vector<Pixel>& pixels, int rows, int cols) {
	cv::Mat image(rows, cols, CV_8UC1); //grayscale
	for (const auto& pixel : pixels) {
		image.at<uchar>(pixel.y, pixel.x) = pixel.value;
	}
	return image;
} */

/* 
static std::vector<Pixel> convertMatToPixels(const cv::Mat& image) {
	std::vector<Pixel> pixels;
	pixels.reserve(image.rows * image.cols);

	for (int y = 0; y < image.rows; ++y) {
		for (int x = 0; x < image.cols; ++x) {
			int value = image.at<uchar>(y, x);
			pixels.emplace_back(value, x, y);
		}
	}
	return pixels;
}
 */



#include <opencv2/opencv.hpp>
#include <random>

void fillUniform(cv::Mat& region) {
    cv::randu(region, cv::Scalar(0), cv::Scalar(256)); // Valori uniforme între 0 și 255
}

void fillGradient(cv::Mat& region) {
    for (int i = 0; i < region.rows; ++i) {
        uchar value = static_cast<uchar>((255.0 / region.rows) * i);
        for (int j = 0; j < region.cols; ++j) {
            region.at<uchar>(i, j) = value;
        }
    }
}

void fillGaussian(cv::Mat& region) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> d(128, 30);

    for (int i = 0; i < region.rows; ++i) {
        for (int j = 0; j < region.cols; ++j) {
            uchar value = static_cast<uchar>(std::min(std::max(int(d(gen)), 0), 255));
            region.at<uchar>(i, j) = value;
        }
    }
}

void fillConstant(cv::Mat& region, uchar value) {
    region.setTo(cv::Scalar(value));
}

cv::Mat generateMatrixWithVariableRegions(size_t size, size_t regionsX, size_t regionsY) 
{
    cv::Mat image(size, size, CV_8UC1, cv::Scalar(0));

    size_t regionWidth = size / regionsX;
    size_t regionHeight = size / regionsY;

    for (size_t y = 0; y < regionsY; ++y) {
        for (size_t x = 0; x < regionsX; ++x) {
            cv::Rect regionRect(x * regionWidth, y * regionHeight, regionWidth, regionHeight);
            cv::Mat region = image(regionRect);

            switch ((x + y) % 4) { // Exemplu de schemă
                case 0: fillUniform(region); break;
                case 1: fillGradient(region); break;
                case 2: fillGaussian(region); break;
                case 3: fillConstant(region, 128); break;
            }
        }
    }

    return image;
}

cv::Mat generateMatrix3C(size_t size) 
{
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