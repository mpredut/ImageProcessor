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
    IImage<T>& image; //make const
    std::vector<PixelCoord> globalHeap;
    std::mutex heapMutex;


    // Comparator for the heap, used to maintain pixels with the highest values.
    struct ComparePixelVal {
        const IImage<T>& img;
        ComparePixelVal(const IImage<T>& image) : img(image) {}

        bool operator()(const PixelCoord& p1, const PixelCoord& p2) const {
            return img.getPixelValue(p1.x, p1.y) > img.getPixelValue(p2.x, p2.y);
        }
    };

    struct ComparePixelValAndCoord {
        const IImage<T>& img;
        ComparePixelValAndCoord(const IImage<T>& image) : img(image) {}

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
    ImageProcessor(IImage<T>& img) : image(img) {}//toto make const



std::vector<PixelCoord> processImage(size_t topN) {
    std::vector<PixelCoord> v;
    v = processImageParallel(topN);
/* 
    static int c = 0;c++;
    if (c == 5) exit(0); */

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

   std::sort(v.begin(), v.end(),  ComparePixelVal(image));

    if (v.size() > topN) {
        v.erase(v.begin() + topN, v.end());
    }

    return v;
};



std::vector<PixelCoord> processImageHeap(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelVal comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);
    for (int y = 0; y < image.rows(); ++y) {
        __builtin_prefetch(image.getPixelPtr());
        for (int x = 0; x < image.cols(); ++x) {
            if (__builtin_expect(v.size() < topN, 1)) {
                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);
            } else {
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


std::vector<PixelCoord> processImageHeapCopy(size_t topN) {
    std::vector<PixelAll> v;
    //ComparePixelVal comp(image);
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);
    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                T pixelValue = image.getPixelValue(x, y);
                PixelAll currentPixel(x, y, pixelValue);
                v.emplace_back(currentPixel); //   v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), ComparePixelValAndCoordCopy());
            } else {
                T pixelValue = image.getPixelValue(x, y);
                //T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
                T heapMinValue = v.front().value;
                if (pixelValue > heapMinValue) {
                    std::pop_heap(v.begin(), v.end(), ComparePixelValAndCoordCopy());
                    v.pop_back();

                    v.emplace_back(PixelAll{x, y, pixelValue});
                    std::push_heap(v.begin(), v.end(), ComparePixelValAndCoordCopy());
                }
            }
        }
    }

    // Overhead added here 
    std::vector<PixelCoord> result;
    std::transform(v.begin(), v.end(), std::back_inserter(result), [](const PixelAll& ac) {
        return PixelCoord(ac.x, ac.y); // Sau orice logică de conversie necesară
    });

return result;
}


std::vector<PixelCoord> processImageHeapNextPixel(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelVal comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);
    image.moveToStart();

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            //T pixelValue = image.getNextPixelValue(); 
            if (v.size() < topN) {
                //PixelAll currentPixel(x, y, pixelValue);
                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);
            } else {
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
    }

    return v; 
}

std::vector<PixelCoord> processImageHeapNextPixelCopy(size_t topN) {
    std::vector<PixelAll> v;
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);
    image.moveToStart();

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getNextPixelValue(); 
            if (v.size() < topN) {
                PixelAll currentPixel(x, y, pixelValue);
                v.emplace_back(currentPixel);
                std::push_heap(v.begin(), v.end(), ComparePixelValAndCoordCopy());
            } else {
                T heapMinValue = v.front().value;
                if (pixelValue > heapMinValue) {
                    std::pop_heap(v.begin(), v.end(), ComparePixelValAndCoordCopy());
                    v.pop_back();
                    
                    v.emplace_back(PixelAll{x, y, pixelValue});
                    std::push_heap(v.begin(), v.end(), ComparePixelValAndCoordCopy());
                }
            }
        }
    }


    std::vector<PixelCoord> result;
    std::transform(v.begin(), v.end(), std::back_inserter(result), [](const PixelAll& ac) {
        return PixelCoord(ac.x, ac.y); 
    });

    return result; 
}


std::vector<PixelCoord> processImageHeapUnrolling(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelVal comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);

    // Calculate the number of full pixels to add
    size_t completeRows = topN / image.cols();
    size_t remainingPixels = topN % image.cols();

    // Add pixels from complete rows
    for (size_t y = 0; y < completeRows; ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            v.emplace_back(x, y);
        }
    }

    // Add the remaining pixels from the next row
    for (size_t x = 0; x < remainingPixels; ++x) {
        v.emplace_back(x, completeRows);
    }

    // Initialize the heap with the added pixels
    std::make_heap(v.begin(), v.end(), comp);

    // Process the rest of the image, if there is any
    size_t y = completeRows;
    for (size_t x = remainingPixels; x < image.cols(); ++x) {
        T pixelValue = image.getPixelValue(x, y);
        T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
        if (pixelValue > heapMinValue) {
            std::pop_heap(v.begin(), v.end(), comp);
            v.pop_back(); 
            v.emplace_back(x, y);
            std::push_heap(v.begin(), v.end(), comp);
        }
    }

    // Process the rest of the image, if there is any
    for (size_t y = completeRows + 1; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
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



std::vector<PixelCoord> processImageHeapBest(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelVal comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);

    // Calculate the number of full pixels to add
    size_t completeRows = topN / image.cols();
    size_t remainingPixels = topN % image.cols();

    // Add pixels from complete rows
    for (size_t y = 0; y < completeRows; ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            v.emplace_back(x, y);
        }
    }

    // Add the remaining pixels from the next row
    for (size_t x = 0; x < remainingPixels; ++x) {
        v.emplace_back(x, completeRows);
    }

    // Initialize the heap with the added pixels
    std::make_heap(v.begin(), v.end(), comp);

    //before processs to next pixels jump image pointer to correct position
    image.moveToStart(topN);
    VectorImage<T>* derivedImg = dynamic_cast<VectorImage<T>*>(&image);
    if (derivedImg) {
        derivedImg->moveToStart(topN);
    } 
    /*
        Because heap is full, compare and possibly replace the min heap
    */ 
    // Process the rest of the row, if there is any
    T heapMinValue = 0;
    T pixelValue = 0;
    size_t y = completeRows;
    for (size_t x = remainingPixels; x < image.cols(); ++x) {
        pixelValue = image.getNextPixelValue(); 
        heapMinValue = image.getPixelValue(v.front().x, v.front().y);
        if (pixelValue > heapMinValue) {
            std::pop_heap(v.begin(), v.end(), comp);
            v.pop_back(); 
            v.emplace_back(x, y);
            std::push_heap(v.begin(), v.end(), comp);
        }
    }

    bool heap_updated = true;
    // Process the rest of the iamge from (completeRows + 1) row to the end, if there is any
    for (size_t y = completeRows + 1; y < image.rows(); ++y) {
        __builtin_prefetch(image.getPixelPtr());
        for (size_t x = 0; x < image.cols(); ++x) {
            pixelValue = image.getNextPixelValue(); //T pixelValue = image.getPixelValue(x, y);
            if(heap_updated) {
                heapMinValue = image.getPixelValue(v.front().x, v.front().y);
            }
            // compare and possibly replace the min heap
             if (__builtin_expect(pixelValue > heapMinValue, 0)) {
                std::pop_heap(v.begin(), v.end(), comp);
                v.pop_back(); 
                v.emplace_back(x, y);
                std::push_heap(v.begin(), v.end(), comp);
                heap_updated = true;
            } else {
                heap_updated  = false;
            }
        }
    }
    
    return v;
}



std::vector<PixelCoord> processImageHeapBest1(size_t topN) {
    std::vector<PixelCoord> v;
    ComparePixelVal comp(image);

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    v.reserve(topN);

    // Calculate the number of full pixels to add
    size_t completeRows = topN / image.cols();
    size_t remainingPixels = topN % image.cols();

    // Add pixels from complete rows
    for (size_t y = 0; y < completeRows; ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            v.emplace_back(x, y);
        }
    }

    // Add the remaining pixels from the next row
    for (size_t x = 0; x < remainingPixels; ++x) {
        v.emplace_back(x, completeRows);
    }

    // Initialize the heap with the added pixels
    std::make_heap(v.begin(), v.end(), comp);

    //before processs to next pixels jump image pointer to correct position
    image.moveToStart(topN);
    VectorImage<T>* derivedImg = dynamic_cast<VectorImage<T>*>(&image);
    if (derivedImg) {
        derivedImg->moveToStart(topN);
    }
    /*
        Because heap is full, compare and possibly replace the min heap
    */ 
    // Process the rest of the row, if there is any
    size_t y = completeRows;
    for (size_t x = remainingPixels; x < image.cols(); ++x) {
        T pixelValue = image.getNextPixelValue(); 
        T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
        if (pixelValue > heapMinValue) {
            std::pop_heap(v.begin(), v.end(), comp);
            v.pop_back(); 
            v.emplace_back(x, y);
            std::push_heap(v.begin(), v.end(), comp);
        }
    }

    // Process the rest of the image from (completeRows + 1) row to the end, if there is any
    for (size_t y = completeRows + 1; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getNextPixelValue(); //T pixelValue = image.getPixelValue(x, y);
            T heapMinValue = image.getPixelValue(v.front().x, v.front().y);
            // compare and possibly replace the min heap
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

void processSubImage(std::vector<PixelCoord>& v, size_t topN, size_t startY, size_t endY) {
    ComparePixelVal comp(image);

    
    //2*topn  + (n - topn) * (2 * log (topn)) 
    for (size_t y = startY; y < endY; ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            if (v.size() < topN) {
                v.push_back({x, y});
                if (v.size() == topN) {
                    std::make_heap(v.begin(), v.end(), comp);
                }
                //std::push_heap(v.begin(), v.end(), comp);
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


void processSubImageSharedHeap(size_t topN, size_t startY, size_t endY, ComparePixelVal& comp) {
        for (size_t y = startY; y < endY; ++y) {
            for (size_t x = 0; x < image.cols(); ++x) {
                auto pixelValue = image.getPixelValue(x, y);

                std::lock_guard<std::mutex> lock(heapMutex);
                if (globalHeap.size() < topN) {
                    globalHeap.push_back({x, y});
                    if (globalHeap.size() == topN) {
                        std::make_heap(globalHeap.begin(), globalHeap.end(), comp);
                    }
                } else {
                    auto heapMinValue = image.getPixelValue(globalHeap.front().x, globalHeap.front().y);
                    if (pixelValue > heapMinValue) {
                        std::pop_heap(globalHeap.begin(), globalHeap.end(), comp);
                        globalHeap.pop_back();

                        globalHeap.push_back({x, y});
                        std::push_heap(globalHeap.begin(), globalHeap.end(), comp);
                    }
                }
            }
        }
    }


std::vector<PixelCoord> processImageParallel(size_t topN) {

    size_t numThreads = std::thread::hardware_concurrency();
    std::cout << "Num threads " << numThreads << std::endl;

    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    std::vector<std::vector<PixelCoord>> localHeaps(numThreads);
    std::vector<std::thread> threads(numThreads);
    ComparePixelVal comp(image);

    for(auto& lh : localHeaps) {
        lh.reserve(topN);
    }

    size_t rowsPerThread = image.rows() / numThreads;

    for (size_t i = 0; i < numThreads; ++i) {
        size_t startY = i * rowsPerThread;
        size_t endY = (i + 1) * rowsPerThread;
        if (i == numThreads - 1) {
            endY = image.rows(); // last thread get rest lines
        }

        //threads.emplace_back(this->processSubImage, std::ref(localHeaps[i]), topN, startY, endY, comp);
        threads[i] = std::thread(std::bind(&ImageProcessor::processSubImage, this, std::ref(localHeaps[i]), topN, startY, endY));
        //threads[i] = std::thread(std::bind(&ImageProcessor::processSubImageSharedHeap, this, topN, startY, endY, comp));
    }

    for (auto& thread : threads) {
        thread.join();
    }



  auto start = std::chrono::high_resolution_clock::now();

   std::vector<PixelCoord> finalHeap;

// Swap with the first local heap if it exists
if (!localHeaps.empty()) {
    finalHeap.swap(localHeaps.front());
}

// Set an iterator to the beginning of the second local heap if it exists, or to the end if not
auto heapIt = localHeaps.begin() + (localHeaps.size() > 1 ? 1 : 0);
auto coordIt = (heapIt != localHeaps.end()) ? heapIt->begin() : std::vector<PixelCoord>::iterator();

// Fill `finalHeap` up to size topN if necessary
while (heapIt != localHeaps.end() && finalHeap.size() < topN) {
    // Add elements from local heaps until we reach topN
    for (; heapIt != localHeaps.end() && coordIt != heapIt->end() && finalHeap.size() < topN; ++coordIt) {
        finalHeap.push_back(*coordIt);
        if (coordIt == heapIt->end() - 1) {
            // Move to the next local heap if we have reached the end of the current one
            ++heapIt;
            coordIt = (heapIt != localHeaps.end()) ? heapIt->begin() - 1 : std::vector<PixelCoord>::iterator();
        }
    }
}

// Make `finalHeap` a heap after adding the initial elements
if (!finalHeap.empty()) {
    std::make_heap(finalHeap.begin(), finalHeap.end(), comp);
}

// Continue to process the remaining elements from the remaining local heaps
for (; heapIt != localHeaps.end(); ++heapIt, coordIt = heapIt->begin()) {
    for (; coordIt != heapIt->end(); ++coordIt) {
        if (comp(*coordIt, finalHeap.front())) {
            finalHeap.push_back(*coordIt);
            std::push_heap(finalHeap.begin(), finalHeap.end(), comp);
            std::pop_heap(finalHeap.begin(), finalHeap.end(), comp);
            finalHeap.pop_back(); // Keep the size at topN
        }
    }
}






/*     // Combine local heaps in one global
    std::vector<PixelCoord> finalHeap;
    for (const auto& heap : localHeaps) {
        for (const auto& coord : heap) {
            finalHeap.push_back(coord);
            if(finalHeap.size() == topN) {
                std::make_heap(finalHeap.begin(), finalHeap.end(), comp);
            }
            //std::push_heap(finalHeap.begin(), finalHeap.end(), comp);
            if (finalHeap.size() > topN) {//
                std::push_heap(finalHeap.begin(), finalHeap.end(), comp);
                std::pop_heap(finalHeap.begin(), finalHeap.end(), comp);
                finalHeap.pop_back();
            }
        }
    } */
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> execTime = end - start;
  
   std::cout << " Intermediar ExecTime: " << execTime.count() << " ms " << std::endl;

    //std::sort_heap(finalHeap.begin(), finalHeap.end(),  ComparePixel(image));
    
    //std::sort_heap(v.begin(), v.end(), ComparePixel());
    //std::sort(finalHeap.begin(), finalHeap.end(), comp);

    return finalHeap;
    

  // return globalHeap;
}


std::vector<PixelCoord> processImageParallelV2(size_t topN) {
        size_t numThreads = std::thread::hardware_concurrency();
        std::cout << "Num threads " << numThreads << std::endl;

        if (topN <= 0 || image.size() < 1) {
            std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
            return {};
        }
        topN = std::min(topN, image.size()); 

        //globalHeap.reserve(topN);

        std::vector<std::thread> workerThreads(numThreads);
        size_t rowsPerThread = image.rows() / numThreads;

        for (size_t i = 0; i < numThreads; ++i) {
            size_t startY = i * rowsPerThread;
            size_t endY = (i + 1) * rowsPerThread;
            if (i == numThreads - 1) {
                endY = image.rows(); // Ensure the last thread processes the remaining rows
            }

            workerThreads[i] = std::thread(&ImageProcessor::workerFunction, this, topN, startY, endY);
        }

        // Wait for processing threads to finish
        for (auto& t : workerThreads) {
            t.join();
        }

        return globalHeap;
    }


    void workerFunction(size_t topN, size_t startY, size_t endY) {
        std::vector<PixelCoord> localHeap;
        localHeap.reserve(topN);
        ComparePixelVal comp(image);
        processSubImage(localHeap, topN, startY, endY);

        std::lock_guard<std::mutex> guard(heapMutex);
        if (globalHeap.empty()) {
            // Swap directly if the global heap is empty
            globalHeap.swap(localHeap);
            //std::make_heap(globalHeap.begin(), globalHeap.end(), comp);
        } else {
            // Otherwise, merge localHeap into globalHeap
            for (const auto& coord : localHeap) {
                if (globalHeap.size() < topN) {
                    globalHeap.push_back(coord);
                    std::push_heap(globalHeap.begin(), globalHeap.end(), comp);
                } else if (comp(coord, globalHeap.front())) {
                    std::pop_heap(globalHeap.begin(), globalHeap.end(), comp);
                    globalHeap.pop_back();

                    globalHeap.push_back(coord);
                    std::push_heap(globalHeap.begin(), globalHeap.end(), comp);
                }
            }
        }
    }



std::vector<PixelCoord> processImagePQ( size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

//    std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelVal> pq(ComparePixelVal(image));
    std::priority_queue<PixelCoord, std::vector<PixelCoord>, ComparePixelVal> pq{ComparePixelVal(image)};


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

  //  std::reverse(v.begin(), v.end());

    return v;
}



std::vector<PixelCoord> processImageCS(size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

     // get the max
    //decltype(image.getPixelValue(0, 0))  maxPixelValue = 0; //the sise of that depend on Image
    T maxPixelValue = 0;

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            if (pixelValue > maxPixelValue) {
                maxPixelValue = pixelValue;
            }
        }
    }

    // use the max valuse to build buckets
    std::vector<std::vector<PixelCoord>> buckets(maxPixelValue + 1);

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            buckets[pixelValue].emplace_back(x, y);
        }
    }

    std::vector<PixelCoord> topPixels;
    topPixels.reserve(topN);

    // Selectează top N pixeli cu cele mai mari valori
    for (auto val = maxPixelValue; val >= 0 && topPixels.size() < topN; --val) {
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

    std::unordered_map<T, std::vector<PixelCoord>> buckets;

    // build buckets using hashmap
    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            buckets[pixelValue].emplace_back(x, y);
        }
    }

    std::vector<PixelCoord> topPixels;
    topPixels.reserve(topN);

    // We traverse the buckets in descending order of the keys to select the top N pixels with the highest values
    std::vector<T> sortedKeys;
    for (const auto& pair : buckets) {
        sortedKeys.push_back(pair.first);
    }
    std::sort(sortedKeys.begin(), sortedKeys.end(), std::greater<unsigned short>());

    for (const auto& key : sortedKeys) {
        for (const auto& coord : buckets[key]) {
            if (topPixels.size() < topN) {
                topPixels.push_back(coord);
            } else {
                break; // We stop the loop when we have collected enough pixels.
            }
        }
        if (topPixels.size() >= topN) {
            break; //
        }
    }

    return topPixels;
}

struct PixelAll {
    size_t x, y;
    T value;

    PixelAll(size_t x, size_t y, T value) : x(x), y(y), value(value) {}

// descending order
bool operator<( const PixelAll& rhs) const {
    return this->value < rhs.value;
};
};

struct ComparePixelValAndCoordCopy {
    bool operator()(const PixelAll& a, const PixelAll& b) const {
        if (a.value != b.value) 
            return a.value > b.value; // descending by value
        if (a.x != b.x) 
            return a.x < b.x;  // if values are equal, sort ascending by x
        return a.y < b.y;  // then by y, pentru a asigura unicitatea
    }
};



std::vector<PixelCoord> convertSetToVector(const std::set<PixelAll, ComparePixelValAndCoordCopy>& topPixels) {
    std::vector<PixelCoord> result;
    for (const auto& item : topPixels) {
        result.emplace_back(item.x, item.y);
    }
    return result;
}

std::vector<PixelCoord> processImageSetCopy(size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    std::set<PixelAll, ComparePixelValAndCoordCopy> topPixels;

    //topPixels.reserve(topN);

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            PixelAll currentPixel(x, y, pixelValue);

            if (topPixels.size() < topN) {
                topPixels.insert(currentPixel);
            } else {
                // If the current element is greater than the smallest in the set (the last element)
                auto itLowest = topPixels.begin(); // The element with the smallest value is the first
                if (currentPixel.value > itLowest->value) {
                    topPixels.erase(itLowest); // Remove the smallest element
                    topPixels.insert(currentPixel); // Insert the new element
                }
            }
        }
    }

    return convertSetToVector(topPixels);
}



std::vector<PixelCoord> processImageSet(size_t topN) {
    if (topN == 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    std::set<T> topPixelValues; 
    std::unordered_map<T, std::vector<PixelCoord>> pixelValueToCoords;
    size_t totalCoords = 0;

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            if (totalCoords < topN) {
                topPixelValues.insert(pixelValue);
                pixelValueToCoords[pixelValue].push_back({x, y});
                totalCoords++;
            } else if (pixelValue > *topPixelValues.begin()) {
              
                auto itLowest = topPixelValues.begin();
                if (pixelValueToCoords[*itLowest].size() > 1) {
                    pixelValueToCoords[*itLowest].pop_back();
                } else {
                    //pixelValueToCoords.erase(*itLowest);
                    topPixelValues.erase(itLowest);
                }
                topPixelValues.insert(pixelValue);
                pixelValueToCoords[pixelValue].push_back({x, y});
            }
        }
    }

    std::vector<PixelCoord> result;
    for (const auto& val : topPixelValues) {
        const auto& coords = pixelValueToCoords[val];
        result.insert(result.end(), coords.begin(), coords.end());
        //if (result.size() >= topN) break;
    }

    return result;
}


std::vector<PixelCoord> processImageSetOld(size_t topN) {
    if (topN <= 0 || image.size() < 1) {
        std::cout << "Invalid input: topN is <= 0 or image has no pixels.\n";
        return {};
    }
    topN = std::min(topN, image.size());

    ComparePixelVal comp(image);
    std::set<PixelCoord, ComparePixelVal> topPixels{comp}; 


    //topPixels.reserve(topN);

    for (size_t y = 0; y < image.rows(); ++y) {
        for (size_t x = 0; x < image.cols(); ++x) {
            T pixelValue = image.getPixelValue(x, y);
            //PixelAll currentPixel(x, y, pixelValue);
            if (topPixels.size() < topN) {
                topPixels.insert({x,y});
            } else {
                auto itLowest = topPixels.begin(); // The element with the smallest value is the first one
                T pixelLowestValue = image.getPixelValue(itLowest->x, itLowest->y);
                if (pixelValue > pixelLowestValue) {
                    topPixels.erase(itLowest); 
                    topPixels.insert({x,y});
                }
            }
        }
    }

     std::vector<PixelCoord> v(topPixels.begin(), topPixels.end());

    return v;
}


/* 
//
std::shared_ptr<IImageProcessor> createImageProcessor(std::shared_ptr<IImageWrapper> wrapper) {
    cv::Mat img = wrapper->getImage();
    
    switch(img.depth()) {
        case CV_8U:
            return std::make_shared<ImageProcessor<uint8_t>>(wrapper);
        case CV_16U:
            return std::make_shared<ImageProcessor<uint16_t>>(wrapper);
        default:
            throw std::runtime_error("Unsupported image type.");
    }
}*/

};//end */
