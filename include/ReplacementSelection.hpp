#pragma once
#include <vector>
#include <string>
#include "BinaryFileIO.hpp"

class ReplacementSelection {
public:
    static std::vector<std::string> generateRuns(const std::string& inputFile, int B, size_t blockSize, IOStats& stats, bool verbose);
};