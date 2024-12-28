#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <chrono>
#include <sstream>
#include <vector>
#include "../include/BPlusTree2.h" // Include your B+ Tree implementation header file

using namespace std;

void testInsertion(int order, const string& filename) {
    cout << "Testing B+ Tree with order " << order << endl;

    std::ifstream infile("key_value_pairs.txt");
    if (!infile) {
        std::cerr << "Failed to open key_value_pairs.txt" << std::endl;
        return;
    }

    BPlusTree<std::string, int> bpt(order); 
    std::map<std::string, int> std_map;
    std::ostringstream buffer;

    std::string key;
    int value;
    int i = 0;
    while (infile >> key >> value) {
        i += 1;
        if (i % 1000 == 0) cout << i << endl;

        // B+ Tree insertion
        auto start_bpt = std::chrono::high_resolution_clock::now();
        bpt.insert(key, value);
        auto end_bpt = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> bpt_insertion_time = end_bpt - start_bpt;
        buffer << bpt_insertion_time.count() << "\n";

        // std::map insertion
        auto start_map = std::chrono::high_resolution_clock::now();
        std_map[key] = value;
        auto end_map = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::micro> map_insertion_time = end_map - start_map;
        buffer << map_insertion_time.count() << "\n";

        // Validation
        if (bpt.search(key) != std_map[key]) {
            std::cerr << "Mismatch found for key: " << key << std::endl;
            return;
        }
    }

    // Write results to file
    std::ofstream logFile(filename);
    logFile << buffer.str();

    infile.close();
    logFile.close();
    std::cout << "Order " << order << " test completed and logged to '" << filename << "'." << std::endl;
}

int main() {
    cout << "Started" << endl;
    vector<int> orders = {4, 10, 20, 50};

    for (int order : orders) {
        string filename = "insertion_times_order_" + to_string(order) + ".txt";
        testInsertion(order, filename);
    }

    return 0;
}
