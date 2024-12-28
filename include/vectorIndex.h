#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <mutex>


// Include the B+ tree header file (from previous implementation, modified KeyType to float)
#include "./bplustree4.h"

// Include hnswlib
#include "../hnswlib/hnswlib/hnswlib.h"


class VectorIndex {
public:
    VectorIndex(int order) 
        : tree(order), dimension(0), hnswIndex(nullptr), space(nullptr), hnswM(16), hnswEfConstruction(200), hnswEfSearch(200) 
    {}

    void insert(const std::vector<float>& vec, float s) {
        if (vec.empty()) {
            throw std::invalid_argument("Cannot insert empty vector");
        }

        if (dataVectors.empty()) {
            dimension = (int)vec.size();
            space = new hnswlib::L2Space(dimension);
            size_t maxElements = 100000; 
            hnswIndex = new hnswlib::HierarchicalNSW<float>(space, maxElements, hnswM, hnswEfConstruction);
            hnswIndex->setEf(hnswEfSearch);
        } else {
            if ((int)vec.size() != dimension) {
                throw std::invalid_argument("All vectors must have the same dimension");
            }
        }

        int idx = (int)dataVectors.size();
        dataVectors.push_back(vec);
        sValues.push_back(s);
        tree.insert(s, idx);

        hnswIndex->addPoint(dataVectors[idx].data(), idx);
    }

    std::vector<int> query(const std::vector<float>& v, int k, float Smin, float Smax, int O = 1000) {
        if (hnswIndex == nullptr || dataVectors.empty()) {
            return {};
        }

        if ((int)v.size() != dimension) {
            throw std::invalid_argument("Query vector dimension does not match index dimension");
        }

        int count = tree.countInRange(Smin, Smax);
        if (count <= 0) {
            return {};
        }

        if (count < O) {
            // Use the B+ tree's rangeQuery directly
            std::vector<int> candidates = tree.rangeQuery(Smin, Smax);

            std::vector<std::pair<float,int>> distIndex;
            distIndex.reserve(candidates.size());
            for (int idx : candidates) {
                float dist = euclideanDistSquared(v, dataVectors[idx]);
                distIndex.push_back({dist, idx});
            }
            std::sort(distIndex.begin(), distIndex.end(), [](auto& a, auto& b){
                return a.first < b.first;
            });

            std::vector<int> result;
            for (int i = 0; i < k && i < (int)distIndex.size(); i++) {
                result.push_back(distIndex[i].second);
            }
            return result;
        } else {
            std::vector<int> annCandidates = approximateNearestNeighbors(v, O);
            std::vector<std::pair<float,int>> distIndex;
            distIndex.reserve(annCandidates.size());
            for (int idx : annCandidates) {
                float sVal = sOfIndex(idx);
                if (sVal >= Smin && sVal <= Smax) {
                    float dist = euclideanDistSquared(v, dataVectors[idx]);
                    distIndex.push_back({dist, idx});
                }
            }

            std::sort(distIndex.begin(), distIndex.end(), [](auto& a, auto& b){
                return a.first < b.first;
            });

            std::vector<int> result;
            for (int i = 0; i < k && i < (int)distIndex.size(); i++) {
                result.push_back(distIndex[i].second);
            }
            return result;
        }
    }

private:
    BPlusTree<float, int> tree;
    std::vector<std::vector<float>> dataVectors;
    std::vector<float> sValues;
    int dimension;

    hnswlib::HierarchicalNSW<float>* hnswIndex;
    hnswlib::SpaceInterface<float>* space;
    int hnswM;
    int hnswEfConstruction;
    int hnswEfSearch;

    double sOfIndex(int idx) const {
        return sValues[idx];
    }

    std::vector<int> approximateNearestNeighbors(const std::vector<float>& query, int O) {
        if (!hnswIndex) {
            return {};
        }
        auto result = hnswIndex->searchKnn(query.data(), O);
        std::vector<int> candidates;
        while (!result.empty()) {
            auto &item = result.top();
            candidates.push_back((int)item.second);
            result.pop();
        }
        std::reverse(candidates.begin(), candidates.end());
        return candidates;
    }

    inline float euclideanDistSquared(const std::vector<float>& a, const std::vector<float>& b) {
        float dist = 0.0f;
        for (size_t i = 0; i < a.size(); i++) {
            float diff = a[i] - b[i];
            dist += diff * diff;
        }
        return dist;
    }

    friend class BPlusTree<float,int>;
};
