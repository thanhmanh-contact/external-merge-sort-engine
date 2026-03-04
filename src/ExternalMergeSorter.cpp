#include "ExternalMergeSorter.hpp"
#include <fstream>
#include <iostream>
#include <queue>
#include <cstdio>
#include <vector>

void ExternalMergeSorter::multiWayMerge(
    std::vector<std::string>& runFiles,
    const std::string& outputFile,
    int B,
    size_t blockSize,
    IOStats& stats,
    bool verbose)
{
    int K = std::max(2, B - 1); // dành 1 buffer cho output
    int pass = 0;

    while (runFiles.size() > 1) {

        pass++;
        stats.passes++;

        std::vector<std::string> nextRuns;

        for (size_t i = 0; i < runFiles.size(); i += K) {

            int endIdx = std::min((int)runFiles.size(), (int)i + K);

            std::string outName =
                (runFiles.size() <= K)
                ? outputFile
                : "pass_" + std::to_string(pass) +
                  "_run_" + std::to_string(i) + ".bin";

            nextRuns.push_back(outName);

            using Element = std::pair<double, int>;
            std::priority_queue<Element,
                                std::vector<Element>,
                                std::greater<Element>> pq;

            int runCount = endIdx - i;
            std::vector<std::ifstream> ins(runCount);

            // ===== BLOCK STATE CHO MỖI RUN =====
            std::vector<std::vector<char>> buffers(runCount,
                                                   std::vector<char>(blockSize));
            std::vector<size_t> bufferPos(runCount, 0);
            std::vector<size_t> bufferCount(runCount, 0);

            auto readNextBlock = [&](int idx) -> bool {
                ins[idx].read(buffers[idx].data(), blockSize);
                std::streamsize bytesRead = ins[idx].gcount();
                if (bytesRead <= 0) return false;

                stats.addRead(); // 1 block read

                bufferCount[idx] = bytesRead / sizeof(double);
                bufferPos[idx] = 0;
                return true;
            };

            auto getNextValue = [&](int idx, double& val) -> bool {
                if (bufferPos[idx] >= bufferCount[idx]) {
                    if (!readNextBlock(idx)) return false;
                }

                double* data =
                    reinterpret_cast<double*>(buffers[idx].data());
                val = data[bufferPos[idx]++];
                return true;
            };

            // ===== Open runs và nạp giá trị đầu =====
            for (int j = 0; j < runCount; j++) {
                ins[j].open(runFiles[i + j], std::ios::binary);

                double val;
                if (getNextValue(j, val)) {
                    pq.push({val, j});
                }
            }

            std::ofstream out(outName, std::ios::binary);

            std::vector<double> outBuffer;
            size_t recordsPerBlock = blockSize / sizeof(double);

            while (!pq.empty()) {

                auto [val, idx] = pq.top();
                pq.pop();

                outBuffer.push_back(val);

                if (outBuffer.size() == recordsPerBlock) {
                    out.write(reinterpret_cast<char*>(outBuffer.data()),
                              blockSize);
                    stats.addWrite(); // 1 block write
                    outBuffer.clear();
                }

                double nextVal;
                if (getNextValue(idx, nextVal)) {
                    pq.push({nextVal, idx});
                }
            }

            // Flush phần còn lại
            if (!outBuffer.empty()) {
                out.write(reinterpret_cast<char*>(outBuffer.data()),
                          outBuffer.size() * sizeof(double));
                stats.addWrite();
                outBuffer.clear();
            }
        }

        // Xoá run cũ
        for (const auto& file : runFiles)
            std::remove(file.c_str());

        runFiles = nextRuns;

        if (verbose) {
            std::cout << "Merge Pass " << pass
                      << " completed.\n";
        }
    }

    if (runFiles.size() == 1 &&
        runFiles[0] != outputFile) {
        std::rename(runFiles[0].c_str(),
                    outputFile.c_str());
    }
}