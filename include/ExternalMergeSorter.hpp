#pragma once
#include <vector>
#include <string>
#include "BinaryFileIO.hpp"

class ExternalMergeSorter {
public:
    static void multiWayMerge(std::vector<std::string>& runFiles, const std::string& outputFile, int B, size_t blockSize, IOStats& stats, bool verbose);
};