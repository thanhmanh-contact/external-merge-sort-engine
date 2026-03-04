#include <iostream>
#include <fstream>
#include <vector>

int main() {
    std::cout << "Enter the number of elements to generate and the values:\n";
    int n;
    std::cin >> n;
    std::vector<double> arr(n);
    for (int i = 0; i < n; i++) { std::cin >> arr[i]; }

    std::ofstream out("input.bin", std::ios::binary);

    if (!out) {
        std::cout << "Unable to open file\n";
        return 1;
    }

    out.write(reinterpret_cast<char*>(arr.data()), n * sizeof(double));

    out.close();
    std::cout << "Data written to file successfully\n";

    return 0;
}