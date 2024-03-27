#pragma once

#include <vector>
#include "image.h"

struct PixelCoord {
    int x, y;
    PixelCoord(int xCoord, int yCoord) : x(xCoord), y(yCoord) {}
};

template<typename T>
std::string toJson(const IImage<T>& image, const std::vector<PixelCoord>& pixels);