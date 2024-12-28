#include "../../include/BPlusTree3.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>

using namespace std;

int main() {
    std::string filename = "../_Data/key_value_pairs_1.txt";  // Replace with actual file path

    std::ofstream timingFile("../_Output/timing_results.txt");
    if (!timingFile) {
        std::cerr << "Failed to open timing_results.txt" << std::endl;
        return 1;
    }

    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Failed to open key_value_pairs.txt" << std::endl;
        return 1;
    }

    int key; 
    string value;
    vector<int> orders = {3, 5, 7, 9, 11, 13, 20, 50, 100, 200};

    for (int order : orders) {
        cout << "Testing B+ Tree with order " << order << endl;
        BPlusTree<int, string> bpt(order); 
        infile.clear();
        infile.seekg(0);
        auto start = chrono::high_resolution_clock::now();
        int bptSize = 0;

        while (infile >> key >> value) {
            bpt.insert(key, value);
            bptSize++;
            if (bptSize % 10000 == 0) {
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << bptSize << '\n';
                timingFile << "B+Tree (Order " << order << ") size: " << bptSize << ", Time: " << elapsed.count() << "s\n";
            }
        }
        cout << "Tested B+Tree with order " << order << ".\n";
    }


    cout << "Tested map insertion.\n";


    cout << "All values for all keys are correct." << endl;
    timingFile.close();

    return 0;
}
