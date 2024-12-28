#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <mutex>

// Include your vector index and B+ tree headers
#include "../../hnswlib/hnswlib/hnswlib.h"
#include "../../include/probabilisticvectorindex.h" // The VectorIndex class as discussed earlier

int main() {
    // Open data.csv
    std::ifstream dataFile("../_Data/_data3.csv");
    if (!dataFile.is_open()) {
        std::cerr << "Error: cannot open data.csv\n";
        return 1;
    }

    std::string line;
    // Read the header of data.csv
    if (!std::getline(dataFile, line)) {
        std::cerr << "Error: data.csv is empty\n";
        return 1;
    }
    // Count columns to deduce D
    {
        std::stringstream ss(line);
        std::string col;
        std::vector<std::string> headers;
        while (std::getline(ss, col, ',')) {
            headers.push_back(col);
        }
        // last column is 's', so dimension D = headers.size() - 1
        int D = (int)headers.size() - 1;

        ProbabilisticVectorIndex index(4); // B+ tree order = 4

        // Read data vectors
        while (std::getline(dataFile, line)) {
            std::stringstream dataSS(line);
            std::vector<float> vec(D);
            for (int i = 0; i < D; i++) {
                if (!std::getline(dataSS, col, ',')) {
                    std::cerr << "Error reading vector value\n";
                    return 1;
                }
                vec[i] = std::stof(col);
            }
            if (!std::getline(dataSS, col, ',')) {
                std::cerr << "Error reading s value\n";
                return 1;
            }
            float s = std::stof(col);

            index.insert(vec, s);
        }
        dataFile.close();

        std::cout << "Data vectors loaded and index built! \n";

        // Now read queries from queries.csv
        std::ifstream queriesFile("../_Data/_queries3.csv");
        if (!queriesFile.is_open()) {
            std::cerr << "Error: cannot open queries.csv\n";
            return 1;
        }

        // Read the header of queries.csv
        if (!std::getline(queriesFile, line)) {
            std::cerr << "Error: queries.csv is empty\n";
            return 1;
        }

        // Count columns to deduce query dimension
        {
            std::stringstream ssq(line);
            std::vector<std::string> qheaders;
            while (std::getline(ssq, col, ',')) {
                qheaders.push_back(col);
            }
            // query format: qv1,...,qvD,k,Smin,Smax,O
            // So dimension D = qheaders.size() - 4
            int queryD = (int)qheaders.size() - 4;

            // Process queries
            int queryCount = 0;
            while (std::getline(queriesFile, line)) {
                queryCount++;
                std::stringstream qs(line);
                std::vector<float> qvec(queryD);
                for (int i = 0; i < queryD; i++) {
                    if (!std::getline(qs, col, ',')) {
                        std::cerr << "Error reading query vector component\n";
                        return 1;
                    }
                    qvec[i] = std::stof(col);
                }
                // Read k,Smin,Smax,O
                if (!std::getline(qs, col, ',')) {std::cerr<<"Error reading k\n";return 1;}
                int k = std::stoi(col);
                if (!std::getline(qs, col, ',')) {std::cerr<<"Error reading Smin\n";return 1;}
                float Smin = std::stof(col);
                if (!std::getline(qs, col, ',')) {std::cerr<<"Error reading Smax\n";return 1;}
                float Smax = std::stof(col);
                if (!std::getline(qs, col, ',')) {std::cerr<<"Error reading O\n";return 1;}
                int O = std::stoi(col);

                // Run query
                std::vector<int> neighbors = index.query(qvec, k, Smin, Smax, O);

                // Print results
                std::cout << "Query " << queryCount << " results: ";
                if (neighbors.empty()) {
                    std::cout << "No neighbors found.\n";
                } else {
                    for (int idx : neighbors) {
                        std::cout << idx << " ";
                    }
                    std::cout << "\n";
                }
            }

            queriesFile.close();
        }
    }

    return 0;
}
