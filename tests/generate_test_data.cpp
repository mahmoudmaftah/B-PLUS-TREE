#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

int main() {
    std::ofstream outfile("test_data.txt");
    if (!outfile) {
        std::cerr << "Error creating file!" << std::endl;
        return 1;
    }

    const long long num_entries = 1000000; // Adjust as needed
    std::srand(std::time(nullptr)); // Seed for random number generation

    for (long long i = 0; i < num_entries; ++i) {
        int key = std::rand(); // Random key
        int value = std::rand(); // Random value
        outfile << key << " " << value << "\n";
    }

    outfile.close();
    std::cout << "Test data file generated successfully!" << std::endl;
    return 0;
}
