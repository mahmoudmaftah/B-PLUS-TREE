#include "../../include/BPlusTree3.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>

using namespace std;

void testBPTInsertion(const string& filename, ofstream& outputFile) {
    vector<int> orders = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 50, 100, 200, 300, 1000, 5000, 10000};
    for (int order : orders) {
        ifstream infile(filename);
        if (!infile) {
            cerr << "Failed to open " << filename << endl;
            return;
        }

        BPlusTree<int, string> bpt(order);
        int key;
        string value;
        auto start = chrono::high_resolution_clock::now();
        int count = 0;

        while (infile >> key >> value) {
            bpt.insert(key, value);
            count++;
            if(count % 1000 == 0) cout << "done with" << count << '\n';
        }

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end - start;
        outputFile << "B+Tree (Order " << order << ") Total Time: " << elapsed.count() << "s\n";
        cout << "Order " << order << " completed with time: " << elapsed.count() << "s\n";
        infile.close();
    }
}

int main() {
    string filename = "../_Data/key_value_pairs_1.txt";
    ofstream outputFile("../_Output/bpt_total_times.txt");
    if (!outputFile) {
        cerr << "Failed to open output file." << endl;
        return 1;
    }

    testBPTInsertion(filename, outputFile);

    outputFile.close();
    cout << "All tests completed. Results saved to bpt_total_times.txt" << endl;
    return 0;
}
