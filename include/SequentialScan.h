#include <iostream>
#include <vector>
#include <string>
#include <utility>

class KeyValueFinder {
private:
    std::vector<std::pair<std::string, int>> keyValuePairs;

public:
    // Method to insert key-value pairs
    void insert(const std::string& key, int value) {
        keyValuePairs.emplace_back(key, value);
    }

    // Sequential search for a key
    int search(const std::string& key) const {
        for (const auto& pair : keyValuePairs) {
            if (pair.first == key) {
                return pair.second;
            }
        }
        throw std::runtime_error("Key not found");
    }

    // Method to display all pairs (for debugging)
    void display() const {
        for (const auto& pair : keyValuePairs) {
            std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
        }
    }
};
