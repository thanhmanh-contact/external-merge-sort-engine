#include <iostream>
#include <string>
#include <cmath>
#include <fstream>
#include "ReplacementSelection.hpp"
#include "ExternalMergeSorter.hpp"
#include "BinaryFileIO.hpp"

int main(int argc, char* argv[]) {

    std::string input = "../examples/input.bin";
    std::string output = "../examples/output.bin";
    int B = 3;
    size_t blockSize = 4096;

    // ===== Parse Arguments =====
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--input" && i + 1 < argc)
            input = argv[++i];

        else if (arg == "--output" && i + 1 < argc)
            output = argv[++i];

        else if (arg == "--buffers" && i + 1 < argc)
            B = std::stoi(argv[++i]);

        else if (arg == "--blocksize" && i + 1 < argc)
            blockSize = std::stoul(argv[++i]);
    }

    if (B <= 1) {
        std::cout << "Error: B must be > 1.\n";
        return 1;
    }

    // ===== Generate sample input nếu chưa có =====
    if (!std::ifstream(input, std::ios::binary)) {
        BinaryFileIO::generateSmallInput(input);
    }

    IOStats stats;
    bool verbose = true;

    std::cout << "========== EXTERNAL MERGE SORT ENGINE ==========\n";
    std::cout << "Buffers (B): " << B << "\n";
    std::cout << "Block Size : " << blockSize << " bytes\n\n";

    // ===== Phase 1: Replacement Selection =====
    auto runs = ReplacementSelection::generateRuns(
        input, B, blockSize, stats, verbose
    );

    // ===== Phase 2: Multi-way Merge =====
    ExternalMergeSorter::multiWayMerge(
        runs, output, B, blockSize, stats, verbose
    );

    // ===== Compute Theoretical IO =====

    // Đếm tổng record
    double totalRecords = 0;
    std::ifstream in(input, std::ios::binary);
    double x;
    while (in.read(reinterpret_cast<char*>(&x), sizeof(double))) {
        totalRecords++;
    }

    size_t recordsPerBlock = blockSize / sizeof(double);
    double N = std::ceil(totalRecords / recordsPerBlock);  // số block

    double fan_in = B - 1;
    double merge_passes = 0;

    if (fan_in > 1 && N > 2 * B) {
        merge_passes = std::ceil(
            std::log(N / (2.0 * B)) / std::log(fan_in)
        );
    }

    double theoretical_IO = 2 * N * (merge_passes + 1);

    // ===== Print Statistics =====
    std::cout << "\n========== IO STATISTICS ==========\n";
    std::cout << "Disk Reads  : " << stats.disk_reads << "\n";
    std::cout << "Disk Writes : " << stats.disk_writes << "\n";
    std::cout << "Total IOs   : " << stats.totalIO() << "\n";
    std::cout << "Total Passes: " << stats.passes << "\n";
    std::cout << "-----------------------------------\n";
    std::cout << "Engineer's Approximation IO: ~ "
              << theoretical_IO << " IOs\n";
    std::cout << "====================================\n";

    return 0;
}