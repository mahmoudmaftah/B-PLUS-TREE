#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <chrono>
#include <sstream>
#include "../include/BPlusTree2.h" // Include your B+ Tree implementation header file

using namespace std;

int main() {
    cout << "Started" << endl;

    std::ifstream infile("key_value_pairs.txt");
    if (!infile) {
        std::cerr << "Failed to open key_value_pairs.txt" << std::endl;
        return 1;
    }

    BPlusTree<std::string, int> bpt(10); // Assuming B+ Tree is templated for key-value types
    std::map<std::string, int> std_map;
    std::ostringstream buffer;

    std::string key;
    int value;
    int i = 0;
    while (infile >> key >> value) {
        i += 1;
        if(i % 1000 == 0) cout << i << endl;
        // Insert into B+ Tree and measure time
        auto start_bpt = std::chrono::high_resolution_clock::now();
        bpt.insert(key, value);
        auto end_bpt = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> bpt_insertion_time = end_bpt - start_bpt;
        buffer << bpt_insertion_time.count() << "\n";

        // Insert into std::map and measure time
        auto start_map = std::chrono::high_resolution_clock::now();
        std_map[key] = value;
        auto end_map = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> map_insertion_time = end_map - start_map;
        buffer << map_insertion_time.count() << "\n";

        // Validate correctness
        if (bpt.search(key) != std_map[key]) {
            std::cerr << "Mismatch found for key: " << key << std::endl;
            return 1;
        }
    }

    // Write all results at once
    std::ofstream logFile("insertion_times.txt");
    logFile << buffer.str();

    infile.close();
    logFile.close();
    std::cout << "Insertion process completed, times logged to 'insertion_times.txt', and correctness verified." << std::endl;

    return 0;
}
