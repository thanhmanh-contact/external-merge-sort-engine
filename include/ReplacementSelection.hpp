#pragma once
#include <vector>
#include <string>
#include "BinaryFileIO.hpp"

class ReplacementSelection {
public:
    // Trả về danh sách tên các run files đã sinh ra
    static std::vector<std::string> generateRuns(const std::string& inputFile, int B, IOStats& stats, bool verbose);
};