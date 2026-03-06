#include "ReplacementSelection.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>

std::vector<std::string> ReplacementSelection::generateRuns(
    const std::string& inputFile,
    int B,
    size_t blockSize,
    IOStats& stats,
    bool verbose)
{
    std::vector<std::string> runFiles;
    std::ifstream in(inputFile, std::ios::binary);
    if (!in || B < 2) return runFiles;

    const size_t recordsPerBlock = blockSize / sizeof(double);
    const size_t heapCapacity = (B - 2) * recordsPerBlock;

    // ===== 1 INPUT BUFFER PAGE =====
    std::vector<double> inputBuffer(recordsPerBlock);
    size_t bufferPos = 0;
    size_t bufferCount = 0;

    auto loadBlock = [&]() -> bool {
        in.read(reinterpret_cast<char*>(inputBuffer.data()), blockSize);
        std::streamsize bytesRead = in.gcount();
        if (bytesRead <= 0) return false;

        stats.addRead();
        bufferCount = bytesRead / sizeof(double);
        bufferPos = 0;
        return true;
    };

    auto getNextValue = [&](double& val) -> bool {
        if (bufferPos >= bufferCount) {
            if (!loadBlock()) return false;
        }
        val = inputBuffer[bufferPos++];
        return true;
    };

    // ===== HEAP + FROZEN nằm trong (B-1) pages =====
    std::vector<double> heap;
    std::vector<double> frozen;

    double val;

    // Fill heap tối đa heapCapacity
    while (heap.size() < heapCapacity && getNextValue(val)) {
        heap.push_back(val);
    }

    std::make_heap(heap.begin(), heap.end(), std::greater<double>());

    int runIdx = 1;
    stats.passes++; // Initial run generation pass

    while (!heap.empty()) {

        std::string runName = "run_" + std::to_string(runIdx) + ".bin";
        runFiles.push_back(runName);

        std::ofstream out(runName, std::ios::binary);

        // ===== 1 OUTPUT BUFFER PAGE =====
        std::vector<double> outputBuffer(recordsPerBlock);
        size_t outPos = 0;

        while (!heap.empty()) {

            std::pop_heap(heap.begin(), heap.end(), std::greater<double>());
            double minVal = heap.back();
            heap.pop_back();

            outputBuffer[outPos++] = minVal;

            if (outPos == recordsPerBlock) {
                out.write(reinterpret_cast<char*>(outputBuffer.data()),
                          blockSize);
                stats.addWrite();
                outPos = 0;
            }

            if (getNextValue(val)) {
                if (val < minVal) {
                    if (heap.size() + frozen.size() < heapCapacity)
                        frozen.push_back(val);
                } else {
                    heap.push_back(val);
                    std::push_heap(heap.begin(), heap.end(),
                                   std::greater<double>());
                }
            }
        }

        // Flush partial output page
        if (outPos > 0) {
            out.write(reinterpret_cast<char*>(outputBuffer.data()),
                      outPos * sizeof(double));
            stats.addWrite();
        }

        // Chuyển frozen → heap cho run tiếp theo
        heap.swap(frozen);
        frozen.clear();

        if (!heap.empty()) {
            std::make_heap(heap.begin(), heap.end(),
                           std::greater<double>());
            runIdx++;
        }
    }

    if (verbose) {
        std::cout << "Initial Runs:\n";
        for (size_t i = 0; i < runFiles.size(); ++i) {
            std::cout << "Run " << i + 1 << ": ";
            BinaryFileIO::printDoubleFile(runFiles[i]);
        }
        std::cout << "\n";
    }

    return runFiles;
}