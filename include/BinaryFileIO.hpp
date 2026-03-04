#pragma once
#include <string>

struct IOStats {
    int disk_reads = 0;
    int disk_writes = 0;
    int passes = 0;

    void addRead() { disk_reads++; }
    void addWrite() { disk_writes++; }
    int totalIO() const { return disk_reads + disk_writes; }
};

class BinaryFileIO {
public:
    static void printDoubleFile(const std::string& filename);
    static void generateSmallInput(const std::string& filename);
};