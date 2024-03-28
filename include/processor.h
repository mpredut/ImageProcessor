#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include<unordered_set>

#include <thread>
#include <mutex>
#include <algorithm>
#include <string>
#include "utils.h"
#include "image.h"

 
template<typename T>
class ImageProcessor {
private:
    const IImage<T>& image; 


    // Comparator for the heap, used to maintain pixels with the highest values.
    struct ComparePixelCoord {
        const IImage<T>& img;
        ComparePixelCoord(const IImage<T>& image) : img(image) {}

        bool operator()(const PixelCoord& p1, const PixelCoord& p2) const {
            return img.getPixelValue(p1.x, p1.y) > img.getPixelValue(p2.x, p2.y);
        }
    };

    struct ComparePixelVal {
        const IImage<T>& img;
        ComparePixelVal(const IImage<T>& image) : img(image) {}

        bool operator()(const PixelCoord& p1, const PixelCoord& p2) const {
            return img.getPixelValue(p1.x, p1.y) > img.getPixelValue(p2.x, p2.y);
        }
    };

    struct ComparePixel {
        const IImage<T>& img;
        ComparePixel(const IImage<T>& image) : img(image) {}

        bool operator()(const PixelCoord& p1, const PixelCoord& p2) const {
            auto val1 = img.getPixelValue(p1.x, p1.y);
            auto val2 = img.getPixelValue(p2.x, p2.y);

            if (val1 == val2) {
                if (p1.y < p2.y) {
                    return true;
                } else if (p1.y == p2.y) {
                    return p1.x < p2.x;
                }
                return false;
            }

            return val1 > val2;
        }
    };



public:
    ImageProcessor(const IImage<T>& img) : image(img) {}


std::vector<PixelCoord> processImage3(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelCoord comp(image);

    v.reserve(topN);
    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);//t
            } else break;
        } 
    }

    //v = processImageSet(topN);
    return v;
}

std::vector<PixelCoord> processImage2(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelCoord comp(image);

    v.reserve(topN);
    
    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                v.emplace_back(x, y);
            } else break;
        }
    }

    std::sort(v.begin(), v.end(), comp);
    //std::make_heap(v.begin(), v.end(), comp);//t


    //v = processImageSet(topN);
    return v;
}

std::vector<PixelCoord> processImageSort( size_t topN) {
    if (topN <= 0) {
        std::cout << "When topN is 0, no pixels are extracted.\n";
        return {};
    }

    std::vector<PixelCoord> v;

    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            v.emplace_back(x, y);
        }
    }

   std::sort(v.begin(), v.end(),  ComparePixelCoord(image));

    if (v.size() > topN) {
        v.erase(v.begin() + topN, v.end());
    }

    return v;
};


std::vector<PixelCoord> processImagePQ( size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

//    std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelCoord> pq(ComparePixelCoord(image));
    std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelCoord> pq{ComparePixelCoord(image)};


    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            PixelCoord currentPixel(x, y);
            if (pq.size() < topN) {
                pq.push(currentPixel);
            } else if (image.getPixelValue(x, y) > image.getPixelValue(pq.top().x, pq.top().y)) {
                pq.pop();
                pq.push(currentPixel);
            }
        }
    }

    std::vector<PixelCoord> v;
    while (!pq.empty() && v.size() < topN) {
        v.push_back(pq.top());
        pq.pop();
    }
    
    // În funcție de cum vrei ca vectorul să fie ordonat, s-ar putea să fie necesar să inversezi ordinea elementelor
    // deoarece priority_queue le scoate în ordinea priorității (de exemplu, de la cel mai mare la cel mai mic)
  //  std::reverse(v.begin(), v.end());

    return v;
}

std::vector<PixelCoord> processImageHeap(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelCoord comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);
    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);//todo!! : move it
                //if(v.size() == topN) std::make_heap(v.begin(), v.end(), comp);
            } else {
                T pixelValue = image.getPixelValue(x, y);
                T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
                
                if (pixelValue > heapMinValue) {
                    std::pop_heap(v.begin(), v.end(), comp);
                    v.pop_back();

                    v.emplace_back(x, y);
                    std::push_heap(v.begin(), v.end(), comp);//todo!! : move it
                }
               // std::push_heap(v.begin(), v.end(), comp);//todo!! : move it
            }
            //std::push_heap(v.begin(), v.end(), comp);//todo!! : move it
        }
    }

    //std::make_heap(v.begin(), v.end(), comp);
    //std::sort_heap(v.begin(), v.end(), comp);
    //std::sort(v.begin(), v.end(),  ComparePixel(image));
    //for (const auto& pixel : v) {
   //     std::cout << "Pixel: (" << pixel.x << ", " << pixel.y << ") = " << image.getPixelValue(pixel.x, pixel.y) << "\n";
   // }
    //std::sort(v.begin(), v.end(), comp);

    return v; 
}


std::vector<PixelCoord> processImage(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelCoord comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);

   
    //std::make_heap(v.begin(), v.end(), comp);

    std::vector<PixelCoord> vv;
     for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            if (vv.size() < topN) {
                vv.push_back({x, y});
            }
        }
     }


 // Calculate the number of full pixels to add
    size_t completeRows = topN / image.cols();
    size_t remainingPixels = topN % image.cols();

    // Add pixels from complete rows
    for (size_t y = 0; y < completeRows; ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            v.emplace_back(x, y);
        }
    }

    // Add the remaining pixels from the next row
    for (size_t x = 0; x < remainingPixels; ++x) {
        v.emplace_back(x, completeRows);
    }

    //compareVectors(v,vv);
    // Initialize the heap with the added pixels
    std::make_heap(v.begin(), v.end(), comp);

    // Process the rest of the image, if there is any
    for (size_t y = completeRows; y < image.rows(); ++y) {
        size_t startCol = (y == completeRows) ? remainingPixels : 0;
        for (size_t x = startCol; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
            //PixelCoord currentPixel(x, y);
            // Only if the heap is full, compare and possibly replace the min heap
            //if (comp(v.front(), currentPixel)) {
             if (pixelValue > heapMinValue) {
                std::pop_heap(v.begin(), v.end(), comp);
                v.pop_back(); 
                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);
            }
        }
    }

    // La final, 'v' conține top 'N' pixeli bazat pe criteriul specificat
    return v;
    

    size_t row = completeRows;
    size_t col = remainingPixels;

    //current row and next cols ... image.cols()
    for (int x = col; x < image.cols(); ++x) {
        T pixelValue = image.getPixelValue(x, row);
        T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
        
        if (pixelValue > heapMinValue) {
            std::pop_heap(v.begin(), v.end(), comp);
            v.pop_back();

            v.emplace_back(x, row);
            std::push_heap(v.begin(), v.end(), comp);
        }
    }

    //next row
    for (int y = row + 1; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
            
            if (pixelValue > heapMinValue) {
                std::pop_heap(v.begin(), v.end(), comp);
                v.pop_back();

                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);
            }
        }
    }

    return v; 
}


/* 
Sequential Memory Access: Pixels in images are generally stored in memory sequentially, line by line.
Accessing them in this order maximizes cache efficiency,
as adjacent data is often preloaded into the cache by the CPU's prefetching mechanisms */

void processSubImage(std::vector<PixelCoord>& v, size_t topN, int startY, int endY) {
    ComparePixelCoord comp(image);

    
    //2*topn  + (n - topn) * (2 * log (topn)) 
    for (int y = startY; y < endY; ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                v.push_back({x, y});
                std::push_heap(v.begin(), v.end(), comp);
            } else {
                T pixelValue = image.getPixelValue(x, y);
                T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
                
                if (pixelValue > heapMinValue) {
                    std::pop_heap(v.begin(), v.end(), comp);
                    v.pop_back();

                    v.push_back({x, y});
                    std::push_heap(v.begin(), v.end(), comp);
                }
            }
        }
    }

    //std::sort_heap(v.begin(), v.end(), ComparePixel());
}



std::vector<PixelCoord> processImageParallel(size_t topN) {

    int numThreads = std::thread::hardware_concurrency();

    if (topN <= 0) {
        std::cout << "When topN is 0, no pixels are extracted.\n";
        return {};
    }

    std::vector<std::vector<PixelCoord>> localHeaps(numThreads);
    std::vector<std::thread> threads(numThreads);
    ComparePixelCoord comp(image);

    // Calculăm numărul de linii pe care fiecare thread trebuie să le proceseze
    int rowsPerThread = image.rows() / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY = (i + 1) * rowsPerThread;
        if (i == numThreads - 1) {
            endY = image.rows(); // Asigurăm că ultimul thread preia restul liniilor
        }

        //threads.emplace_back(this->processSubImage, std::ref(localHeaps[i]), topN, startY, endY, comp);
        threads[i] = std::thread(std::bind(&ImageProcessor::processSubImage, this, std::ref(localHeaps[i]), topN, startY, endY));
    }

    // Așteptăm finalizarea execuției fiecărui thread
    for (auto& thread : threads) {
        thread.join();
    }

    // Combinăm heap-urile locale într-unul global
    std::vector<PixelCoord> finalHeap;
    for (const auto& heap : localHeaps) {
        for (const auto& coord : heap) {
            finalHeap.push_back(coord);
            std::push_heap(finalHeap.begin(), finalHeap.end(), comp);
            if (finalHeap.size() > topN) {
                std::pop_heap(finalHeap.begin(), finalHeap.end(), comp);
                finalHeap.pop_back();
            }
        }
    }

    //std::sort_heap(finalHeap.begin(), finalHeap.end(),  ComparePixel(image));
    
    //std::sort_heap(v.begin(), v.end(), ComparePixel());
    //std::sort(finalHeap.begin(), finalHeap.end(), comp);
    return finalHeap;
}

std::vector<PixelCoord> processImageCS(size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

     // Determină valoarea maximă a pixelilor
    //decltype(image.getPixelValue(0, 0))  maxPixelValue = 0; //the sise of that depend on Image
    T maxPixelValue = 0;

    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            if (pixelValue > maxPixelValue) {
                maxPixelValue = pixelValue;
            }
        }
    }

    // Folosește valoarea maximă găsită pentru a construi buckets
    std::vector<std::vector<PixelCoord>> buckets(maxPixelValue + 1);

    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            buckets[pixelValue].emplace_back(x, y);
        }
    }

    std::vector<PixelCoord> topPixels;
    topPixels.reserve(topN);

    // Selectează top N pixeli cu cele mai mari valori
    for (int val = maxPixelValue; val >= 0 && topPixels.size() < topN; --val) {
        for (const auto& coord : buckets[val]) {
            if (topPixels.size() < topN) {
                topPixels.emplace_back(coord);
            } else {
                break;
            }
        }
    }

    return topPixels;
}


std::vector<PixelCoord> processImageCS_MAP( size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    std::unordered_map<unsigned short, std::vector<PixelCoord>> buckets;

    // Construiește buckets folosind un hashmap
    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            buckets[pixelValue].emplace_back(x, y);
        }
    }

    std::vector<PixelCoord> topPixels;
    topPixels.reserve(topN);

    // Parcurgem buckets în ordinea descrescătoare a cheilor pentru a selecta top N pixeli cu cele mai mari valori
    std::vector<unsigned short> sortedKeys;
    for (const auto& pair : buckets) {
        sortedKeys.push_back(pair.first);
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), std::greater<unsigned short>());

    for (const auto& key : sortedKeys) {
        for (const auto& coord : buckets[key]) {
            if (topPixels.size() < topN) {
                topPixels.push_back(coord);
            } else {
                break; // Oprim bucla când am colectat suficiente pixeli
            }
        }
        if (topPixels.size() >= topN) {
            break; // Oprim bucla exterioară de asemenea
        }
    }

    return topPixels;
}

struct AltComp {
    int x, y;
    unsigned short value;

    AltComp(int x, int y, unsigned short value) : x(x), y(y), value(value) {}

// Comparator pentru a menține elementele în ordine descrescătoare
bool operator<( const AltComp& rhs) const {
    return this->value < rhs.value;
};
};

struct AltCompCompare {
    bool operator()(const AltComp& a, const AltComp& b) const {
        if (a.value != b.value) 
            return a.value > b.value; // Ordine descrescătoare după valoare
        if (a.x != b.x) 
            return a.x < b.x;  // Dacă valorile sunt egale, ordonează crescător după x
        return a.y < b.y;  // Apoi după y, pentru a asigura unicitatea
    }
};


std::vector<PixelCoord> convertSetToVector(const std::set<AltComp, AltCompCompare>& topPixels) {
    std::vector<PixelCoord> result;
    for (const auto& item : topPixels) {
        result.emplace_back(item.x, item.y);
    }
    return result;
}

std::vector<PixelCoord> processImageSet(size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    std::set<AltComp, AltCompCompare> topPixels;

    //topPixels.reserve(topN);

    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            AltComp currentPixel(x, y, pixelValue);

            // Adaugă direct dacă nu am ajuns încă la limita topN
            if (topPixels.size() < topN) {
                topPixels.insert(currentPixel);
            } else {
                // Dacă elementul curent este mai mare decât cel mai mic din set (ultimul element)
                auto itLowest = topPixels.begin(); // Elementul cu cea mai mică valoare este primul
                if (currentPixel.value > itLowest->value) {
                    topPixels.erase(itLowest); // Elimină cel mai mic element
                    topPixels.insert(currentPixel); // Inserează noul element
                }
            }
        }
    }

    return convertSetToVector(topPixels);

    // Convertim setul în vector pentru a returna rezultatul
   // return std::vector<PixelCoord>(topPixels.begin(), topPixels.end());
}



std::vector<PixelCoord> processImageHeapNice(size_t topN) {
    std::vector<AltComp> v;
    //ComparePixelCoord comp(image);
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);
    for (int y = 0; y < image.rows(); ++y) {
        for (int x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                //v.emplace_back(x, y);
                T pixelValue = image.getPixelValue(x, y);
                AltComp currentPixel(x, y, pixelValue);
                v.emplace_back(currentPixel);
                std::push_heap(v.begin(), v.end(), AltCompCompare());
            } else {
                T pixelValue = image.getPixelValue(x, y);
                //T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
                T heapMinValue = v.front().value;
                if (pixelValue > heapMinValue) {
                    std::pop_heap(v.begin(), v.end(), AltCompCompare());
                    v.pop_back();

                    v.emplace_back(AltComp{x, y, pixelValue});
                    std::push_heap(v.begin(), v.end(), AltCompCompare());
                }
            }
        }
    }


std::vector<PixelCoord> result;
std::transform(v.begin(), v.end(), std::back_inserter(result), [](const AltComp& ac) {
    return PixelCoord(ac.x, ac.y); // Sau orice logică de conversie necesară
});

return result;
}

};//end
