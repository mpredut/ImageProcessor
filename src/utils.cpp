
#include <opencv2/opencv.hpp>

#include <string>

#include "utils.h"
#include "image.h"


#include "utils.h"

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

template<typename T>
std::string toJson(const IImage<T>& image, const std::vector<PixelCoord>& pixels) {
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
    json += "] }";
    return json;
}