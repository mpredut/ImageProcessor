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
protected:
    const T* imgStartPtr;   // Pointer to beggining of image data.
    mutable T* imgPtr;      // Pointer to image data.

private:
    IImage() = delete;  
    //IImage() : imgStartPtr(nullptr), imgPtr(nullptr) {}

public:
    explicit IImage(T* ptr) : imgStartPtr(ptr), imgPtr(ptr) {}

    virtual T getPixelValue(int x, int y) const = 0;
    virtual T rows() const = 0;
    virtual T cols() const = 0;
    virtual size_t size() const = 0;

    virtual T getNextPixelValue() {
        //if (imgPtr != nullptr) {
            return *(imgPtr++);
        //}
        return T();
    }

    // reset imgPtr 
    void moveToStart() const {
        imgPtr = const_cast<T*>(imgStartPtr);
    }

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
    }

    virtual ~IImage() = default;
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
template<typename T>
class ImageWrapper : public IImage<T> {
private:
    cv::Mat image;

public:
    ImageWrapper(const cv::Mat& img) 
        : IImage<T>(reinterpret_cast<T*>(img.data)), 
          image(img.isContinuous() ? img : img.clone()) {
        if (!image.isContinuous()) {
            throw std::runtime_error("Image data must be continuous and of type ...");
            //|| image.type() != CV_16U
        }
    }//IImage<uint16_t>(img.isContinuous() ? reinterpret_cast<uint16_t*>(img.data) : nullptr), image(img.isContinuous() ? img : img.clone()) {

    
    T getPixelValue(int x, int y) const {
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
        return image.ptr<uint16_t>(y)[x];
	}

    T rows() const override { return image.rows; }
    T cols() const override { return image.cols; }
    size_t size() const override { return static_cast<size_t>(image.total()); }
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
    std::vector<std::tuple<T, int, int>> data; //toto: make reference
    int max_x = -1;
    int max_y = -1;

    int xstart = 0;
    int ystart = 0;

public:
    VectorImage(const std::vector<std::tuple<T, int, int>>& vec) : IImage<T>(nullptr), data(vec) {
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

    T rows() const override {
        return max_y + 1; // max value + 1 give the max rows
    }
    T cols() const override {
        return max_x + 1; // max value + 1 give the max cols
    }
    size_t size() const override {
        return (rows() * cols());
    };

    // reset imgPtr 
    void moveToStart() {
        xstart = 0; ystart = 0; 
    }


    T getNextPixelValue() {
        if (xstart >= rows() || ystart >= cols()) {
            return T(); 
            throw std::runtime_error("Unsupported pixel value.");
        }
        T value = getPixelValue(ystart, xstart);

        ++ystart;

        if (ystart >= cols()) {
            ystart = 0;
            ++xstart; 
        }

        return value;
    }


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
std::unique_ptr<IImage> createImageWrapper(const cv::Mat& img) {
    switch (img.depth()) {
        case CV_8U:
            return std::make_unique<ImageWrapper<uint8_t>>(img);
        case CV_16U:
            return std::make_unique<ImageWrapper<uint16_t>>(img);
        case CV_32F:
            return std::make_unique<ImageWrapper<float>>(img);
        // Add other cases as necessary
        default:
            throw std::runtime_error("Unsupported image type.");
    }
} */


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
        return std::make_unique<ImageWrapper<uint16_t>>(image);
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



