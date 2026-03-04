#include <iostream>
#include <fstream>

int main() {
    std::ifstream in("input.bin", std::ios::binary);

    double x;

    while (in.read(reinterpret_cast<char*>(&x), sizeof(double))) {
        std::cout << x << " ";
    }

    in.close();
    return 0;
}
