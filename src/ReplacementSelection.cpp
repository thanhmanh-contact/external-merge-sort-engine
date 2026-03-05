#include "ReplacementSelection.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

std::vector<std::string> ReplacementSelection::generateRuns(
    const std::string& inputFile,
    int B,
    size_t blockSize,
    IOStats& stats,
    bool verbose)
{
    std::vector<std::string> runFiles;
    std::ifstream in(inputFile, std::ios::binary);
    if (!in) return runFiles;

    std::vector<double> heap;
    std::vector<double> frozen;

    // ===== BLOCK READ STATE =====
    std::vector<char> buffer(blockSize);
    size_t bufferPos = 0;
    size_t bufferCount = 0;

    auto readNextBlock = [&]() -> bool {
        in.read(buffer.data(), blockSize);
        std::streamsize bytesRead = in.gcount();
        if (bytesRead <= 0) return false;

        stats.addRead(); // 1 block = 1 IO

        bufferCount = bytesRead / sizeof(double);
        bufferPos = 0;
        return true;
    };

    auto getNextValue = [&](double& value) -> bool {
        if (bufferPos >= bufferCount) {
            if (!readNextBlock()) return false;
        }

        double* data = reinterpret_cast<double*>(buffer.data());
        value = data[bufferPos++];
        return true;
    };

    // ===== Fill heap ban đầu =====
    double val;
    while (heap.size() < (size_t)B && getNextValue(val)) {
        heap.push_back(val);
    }

    std::make_heap(heap.begin(), heap.end(), std::greater<double>());
    int runIdx = 1;
    stats.passes++; // Pass 1: Run generation

    while (!heap.empty() || !frozen.empty()) {

        if (heap.empty()) {
            heap = frozen;
            frozen.clear();
            std::make_heap(heap.begin(), heap.end(), std::greater<double>());
            runIdx++;
        }

        std::string runName = "run_" + std::to_string(runIdx) + ".bin";
        if (runFiles.empty() || runFiles.back() != runName) {
            runFiles.push_back(runName);
        }

        std::ofstream out(runName, std::ios::binary);

        // ===== Block write buffer =====
        std::vector<double> outBuffer;
        size_t recordsPerBlock = blockSize / sizeof(double);

        while (!heap.empty()) {

            std::pop_heap(heap.begin(), heap.end(), std::greater<double>());
            double minVal = heap.back();
            heap.pop_back();

            outBuffer.push_back(minVal);

            if (outBuffer.size() == recordsPerBlock) {
                out.write(reinterpret_cast<char*>(outBuffer.data()), blockSize);
                stats.addWrite(); // 1 block write
                outBuffer.clear();
            }

            if (getNextValue(val)) {
                if (val < minVal) {
                    frozen.push_back(val);
                } else {
                    heap.push_back(val);
                    std::push_heap(heap.begin(), heap.end(), std::greater<double>());
                }
            }
        }

        // Flush phần còn lại < 1 block
        if (!outBuffer.empty()) {
            out.write(reinterpret_cast<char*>(outBuffer.data()),
                      outBuffer.size() * sizeof(double));
            stats.addWrite();
            outBuffer.clear();
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