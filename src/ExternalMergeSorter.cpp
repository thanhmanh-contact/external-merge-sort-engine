#include "ExternalMergeSorter.hpp"
#include <fstream>
#include <iostream>
#include <queue>
#include <cstdio>
#include <vector>
#include <algorithm>

void ExternalMergeSorter::multiWayMerge(
    std::vector<std::string>& runFiles,
    const std::string& outputFile,
    int B,
    size_t blockSize,
    IOStats& stats,
    bool verbose)
{
    if (B < 3) {
        std::cerr << "Need at least 3 buffers for multi-way merge.\n";
        return;
    }

    const int fanIn = B - 1;   
    const size_t recordsPerBlock = blockSize / sizeof(double);

    int pass = 0;

    while (runFiles.size() > 1) {

        pass++;
        stats.passes++;

        std::vector<std::string> nextRuns;

        for (size_t i = 0; i < runFiles.size(); i += fanIn) {

            int runCount = std::min((int)runFiles.size() - (int)i, fanIn);

            std::string outName =
                (runFiles.size() <= fanIn)
                ? outputFile
                : "pass_" + std::to_string(pass) +
                  "_run_" + std::to_string(i) + ".bin";

            nextRuns.push_back(outName);

            using Element = std::pair<double, int>;
            std::priority_queue<Element,
                                std::vector<Element>,
                                std::greater<Element>> pq;

            // ===== B-1 INPUT BUFFERS =====
            std::vector<std::ifstream> ins(runCount);
            std::vector<std::vector<double>> inputBuffers(
                runCount,
                std::vector<double>(recordsPerBlock));

            std::vector<size_t> bufferPos(runCount, 0);
            std::vector<size_t> bufferCount(runCount, 0);

            auto loadBlock = [&](int idx) -> bool {
                ins[idx].read(reinterpret_cast<char*>(
                              inputBuffers[idx].data()),
                              blockSize);

                std::streamsize bytesRead = ins[idx].gcount();
                if (bytesRead <= 0) return false;

                stats.addRead();

                bufferCount[idx] = bytesRead / sizeof(double);
                bufferPos[idx] = 0;
                return true;
            };

            auto getNextValue = [&](int idx, double& val) -> bool {
                if (bufferPos[idx] >= bufferCount[idx]) {
                    if (!loadBlock(idx)) return false;
                }
                val = inputBuffers[idx][bufferPos[idx]++];
                return true;
            };

            // ===== OPEN RUNS =====
            for (int j = 0; j < runCount; j++) {
                ins[j].open(runFiles[i + j], std::ios::binary);
                double val;
                if (getNextValue(j, val)) {
                    pq.push({val, j});
                }
            }

            // ===== 1 OUTPUT BUFFER =====
            std::ofstream out(outName, std::ios::binary);
            std::vector<double> outputBuffer(recordsPerBlock);
            size_t outPos = 0;

            while (!pq.empty()) {

                auto [val, idx] = pq.top();
                pq.pop();

                outputBuffer[outPos++] = val;

                if (outPos == recordsPerBlock) {
                    out.write(reinterpret_cast<char*>(
                              outputBuffer.data()),
                              blockSize);
                    stats.addWrite();
                    outPos = 0;
                }

                double nextVal;
                if (getNextValue(idx, nextVal)) {
                    pq.push({nextVal, idx});
                }
            }

            // Flush partial block
            if (outPos > 0) {
                out.write(reinterpret_cast<char*>(
                          outputBuffer.data()),
                          outPos * sizeof(double));
                stats.addWrite();
            }
        }

        // Remove old runs
        for (const auto& f : runFiles)
            std::remove(f.c_str());

        runFiles = nextRuns;

        if (verbose) {
            std::cout << "Merge Pass "
                      << pass << " completed.\n";
        }
    }

    if (runFiles.size() == 1 &&
        runFiles[0] != outputFile) {
        std::rename(runFiles[0].c_str(),
                    outputFile.c_str());
    }
}