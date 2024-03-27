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


std::vector<PixelCoord> processImage(size_t topN) {
    std::vector<PixelCoord> v;

    v = processImageSet(topN);
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
    if (topN <= 0) {
        std::cout << "When topN is 0, no pixels are extracted.\n";
        return {};
    }

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

    if (topN <= 0) {
            std::cout << "When topN is 0, no pixels are extracted.\n";
            return {};
    }

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

/* 
Sequential Memory Access: Pixels in images are generally stored in memory sequentially, line by line.
Accessing them in this order maximizes cache efficiency,
as adjacent data is often preloaded into the cache by the CPU's prefetching mechanisms */

void processSubImage(std::vector<PixelCoord>& v, size_t topN, int startY, int endY) {
    ComparePixelCoord comp(image);

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

std::priority_queue<PixelCoord, std::vector<PixelCoord>, std::greater<PixelCoord>>
	mergeTopPixelQueues(const std::vector<std::priority_queue<PixelCoord, std::vector<PixelCoord>, 
                        std::greater<PixelCoord>>>& pixelQueues, int topN) {
        std::priority_queue<PixelCoord, std::vector<PixelCoord>, std::greater<PixelCoord>> finalQueue;

        for (const auto& queue : pixelQueues) {
            std::priority_queue<PixelCoord, std::vector<PixelCoord>, std::greater<PixelCoord>> tempQueue = queue;
            while (!tempQueue.empty()) {
                finalQueue.push(tempQueue.top());
                tempQueue.pop();

                if (finalQueue.size() > topN) {
                    finalQueue.pop();
                }
            }
        }

        return finalQueue;
}


std::vector<PixelCoord> mergeLocalHeapsUsingPriorityQueue(
        const std::vector<std::vector<PixelCoord>>& localHeaps, size_t topN) {
    ComparePixelCoord comp(image);
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
    if (topN <= 0) {
        std::cout << "When topN is 0, no pixels are extracted.\n";
        return {};
    }

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
    if (topN <= 0) {
        std::cout << "When topN is 0, no pixels are extracted.\n";
        return {};
    }

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
    // Notă: Nu putem parcurge direct un unordered_map în ordine descrescătoare a cheilor, așa că vom colecta cheile separat și le vom sorta.
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
    if (topN <= 0) {
        std::cout << "When topN is 0, no pixels are extracted.\n";
        return {};
    }

    std::set<AltComp, AltCompCompare> topPixels;

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

    if (topN <= 0) {
            std::cout << "When topN is 0, no pixels are extracted.\n";
            return {};
    }

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
