#pragma once

#include <vector>
#include "image.h"

struct PixelCoord {
    size_t x, y;
    PixelCoord(int xCoord, int yCoord) : x(xCoord), y(yCoord) {}
};


template<typename T>
 struct ComparePixelVal {
        const IImage<T>& img;
        ComparePixelVal(const IImage<T>& image) : img(image) {}

        bool operator()(const PixelCoord& p1, const PixelCoord& p2) const {
            return img.getPixelValue(p1.x, p1.y) > img.getPixelValue(p2.x, p2.y);
        }
    };

bool compareVectors(const std::vector<PixelCoord>& vec1, const std::vector<PixelCoord>& vec2);

template<typename T>
std::string toJson(const IImage<T>& image, const std::vector<PixelCoord>& pixels);

