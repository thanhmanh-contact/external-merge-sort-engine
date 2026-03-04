#include "BinaryFileIO.hpp"
#include <iostream>
#include <fstream>
#include <vector>

void BinaryFileIO::printDoubleFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    double val;
    while (in.read(reinterpret_cast<char*>(&val), sizeof(double))) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

void BinaryFileIO::generateSmallInput(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    std::vector<double> data = {1.2, 4.5, 7.8, 0.3, 2.1, 9.4};
    for (double val : data) {
        out.write(reinterpret_cast<const char*>(&val), sizeof(double));
    }
}