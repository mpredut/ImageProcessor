#pragma once 

#include <opencv2/opencv.hpp>

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include <variant>
#include <vector>
#include <tuple>
#include <iostream>
#include <stdexcept>

//Interface
template <typename T>
class IImage {
public:
    virtual T getPixelValue(int x, int y) const = 0;
    virtual int rows() const = 0;
    virtual int cols() const = 0;
    virtual size_t size() const = 0;
    
    //
    // debug only interfaces
    //
     int printImage() const {
        for (int y = 0; y < this->rows(); ++y) {
            for (int x = 0; x < this->cols(); ++x) {
                std::cout << this->getPixelValue(x, y) << " ";
            }
            std::cout << "\n";
        }
        return 0;
    };

    virtual ~IImage() = default;
};


/*
 * The VectorImage class Instance implementation for IImage.
 * It is designed to facilitate image testing and processing by 
 * allowing flexible and easy interaction with image data,
 * even when a full representation of every pixel is not necessary. 
 * It assumes that any pixel not explicitly included in the vector 
 * has a value of 0 (black), indicating a default black background for the image.
 * Image dimensions are determined by identifying 
 * the highest x and y coordinates among the provided pixels, 
 * then adding one to accurately estimate the image's dimensions.
 */

template <typename T>
class VectorImage : public IImage<T> {
private:
    std::vector<std::tuple<T, int, int>> data;
    int max_x = -1;
    int max_y = -1;

public:
    VectorImage(const std::vector<std::tuple<T, int, int>>& vec) : data(vec) {
        for (const auto& [color, x, y] : vec) {
            max_x = std::max(max_x, (int)x);
            max_y = std::max(max_y, (int)y);
        }
    }

    T getPixelValue(int x, int y) const override {
        auto it = std::find_if(data.begin(), data.end(), 
                    [x, y](const std::tuple<T, int, int>& pixel) {
                        return std::get<1>(pixel) == x && std::get<2>(pixel) == y;
                    });
        if (it != data.end()) {
            return std::get<0>(*it); // Return the value of the pixel color.
        }
        return 0; //Assume a black background for pixels that are not specified
    }

    int rows() const override {
        return max_y + 1; // max value + 1 give the max rows
    }
    int cols() const override {
        return max_x + 1; // max value + 1 give the max cols
    }
    size_t size() const override {
        return (rows() * cols());
    };

    //
    //debug only call
    //
    std::vector<std::tuple<T, int, int>> getPixelsSortedByValue() const {
        std::vector<std::tuple<T, int, int>> sortedData(data.begin(), data.end());
        std::sort(sortedData.begin(), sortedData.end(), 
        [](const std::tuple<T, int, int>& a, const std::tuple<T, int, int>& b) {
            return std::get<0>(a) > std::get<0>(b);
        });

        // After sorting, fill in the missing (black) pixels
        if (sortedData.size() < size()) {
            for (int y = 0; y < rows(); ++y) {
                for (int x = 0; x < cols(); ++x) {
                    if (getPixelValue(x, y) == 0) { // Checks if pixel is black/missing
                        sortedData.push_back(std::make_tuple(0, x, y));
                    }
                }
            }
        }
    
        std::cout << "Pixels sorted in descending order by color value:\n";
        for (const auto& pixel : sortedData) {
            auto [color, x, y] = pixel;
            std::cout << "Color: " << color << " at (" << x << ", " << y << ")\n";
        }
        
        return sortedData;
    };
};



/*
 * The ImageWrapper class Instance implementation for IImage.
 * It is designed to facilitate real image manipulation
 * in real-life scenarios. 
 * Utilizes the OpenCV cv::Mat class to handle image data, offering
 * comprehensive support for a wide range of image formats and operations.
 * This integration supporting various image depths including
 *  8-bit, 16-bit, and floating-point representations.
 */


class ImageWrapper : public IImage<uint16_t> {
private:
    cv::Mat image;

public:
    ImageWrapper(const cv::Mat& img) : image(img) {};

	uint16_t getPixelValue(int x, int y) const {
        if (x < 0 || x >= image.cols || y < 0 || y >= image.rows) {
            throw std::out_of_range("Pixel coordinates out of range");
        }
		switch (image.depth()) {
			case CV_8U:
				return image.at<uchar>(y, x);
			case CV_16U:
				return image.at<uint16_t>(y, x);
			case CV_32F:
				return static_cast<int>(image.at<float>(y, x));
			default:
				throw std::runtime_error("Unsupported image depth.");
		}
	}

    int rows() const override { return image.rows; }
    int cols() const override { return image.cols; }
    size_t size() const override { return (rows() * cols()); }
};



/*
 * Base Interface class for image reading operations. 
 * It reading images from a file path or an image sensor.
 */
class ImageReader {
public:
    virtual std::unique_ptr<IImage<uint16_t>> readImage(const std::string& imagePath) {
        throw std::runtime_error("readImage not implemented");
    }
    virtual std::unique_ptr<IImage<uint16_t>> readImageSensor() {
        throw std::runtime_error("readImageSensor not implemented");
    }
    virtual ~ImageReader() = default;
};


/*
 * Particular implementation of ImageReader by reading image using OpenCV.
 * Load an image from a file path as a grayscale image,
 * and wraps it in an ImageWrapper for manipulation.
 */
class OpenCVImageReader : public ImageReader {
public:
    std::unique_ptr<IImage<uint16_t>> readImage(const std::string& imagePath) override {
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        if (image.empty()) {
            throw std::runtime_error("Error loading the image.");
        }
        return std::make_unique<ImageWrapper>(image);
    }
};

/*
 * Factory class for creating instances of ImageReader.
 */
class ImageReaderFactory {
public:
    static std::unique_ptr<ImageReader> createImageReader() {
        return std::make_unique<OpenCVImageReader>();
    }
};

