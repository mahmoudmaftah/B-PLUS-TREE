#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>

class NaiveVectorIndex {
public:
    NaiveVectorIndex() : dimension(0) {}

    // Insert a record: vector and s
    void insert(const std::vector<float>& vec, float s) {
        if (vec.empty()) {
            throw std::invalid_argument("Cannot insert empty vector");
        }

        if (dataVectors.empty()) {
            // First inserted vector defines the dimension
            dimension = (int)vec.size();
        } else {
            if ((int)vec.size() != dimension) {
                throw std::invalid_argument("All vectors must have the same dimension");
            }
        }

        dataVectors.push_back(vec);
        sValues.push_back(s);
    }

    // Query: Given vector v, integer k, and range [Smin, Smax]
    // This is a naive approach:
    // 1. Compute the distance to all vectors.
    // 2. Filter by s in [Smin, Smax].
    // 3. Sort by distance and take top k.
    std::vector<int> query(const std::vector<float>& v, int k, float Smin, float Smax) {
        if (dataVectors.empty()) {
            return {};
        }

        if ((int)v.size() != dimension) {
            throw std::invalid_argument("Query vector dimension does not match index dimension");
        }

        std::vector<std::pair<float, int>> distIndex; 
        distIndex.reserve(dataVectors.size());

        // Compute distances and filter by s
        for (int i = 0; i < (int)dataVectors.size(); i++) {
            float sVal = sValues[i];
            if (sVal >= Smin && sVal <= Smax) {
                float dist = euclideanDistSquared(v, dataVectors[i]);
                distIndex.push_back({dist, i});
            }
        }

        // Sort by distance
        std::sort(distIndex.begin(), distIndex.end(), [](auto& a, auto& b) {
            return a.first < b.first;
        });

        // Take top k
        std::vector<int> result;
        for (int i = 0; i < k && i < (int)distIndex.size(); i++) {
            result.push_back(distIndex[i].second);
        }

        return result;
    }

private:
    std::vector<std::vector<float>> dataVectors;
    std::vector<float> sValues;
    int dimension;

    float euclideanDistSquared(const std::vector<float>& a, const std::vector<float>& b) {
        float dist = 0.0f;
        for (size_t i = 0; i < a.size(); i++) {
            float diff = a[i] - b[i];
            dist += diff * diff;
        }
        return dist;
    }
};
