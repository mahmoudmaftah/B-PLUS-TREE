#include <iostream>
#include <vector>
#include <mutex> // Ensure this is included first!
#include "../hnswlib/hnswlib/hnswlib.h"

using namespace std;

int main() {
    // Dimension of the vectors
    int dim = 4;
    // Number of vectors to insert
    int num_elements = 5;

    // Create the L2 space (Euclidean distance)
    hnswlib::L2Space space(dim);

    // Create a HierarchicalNSW index with capacity for max_elements
    int max_elements = num_elements;
    int M = 16;           // Parameter that defines the number of bi-directional links created for every new element
    int ef_construction = 200; // Higher ef_construction leads to better indexing quality and slower construction
    hnswlib::HierarchicalNSW<float> appr_alg(&space, max_elements, M, ef_construction);

    // Optionally, set ef (size of the dynamic list for the nearest neighbors) at query time
    appr_alg.setEf(50);

    // Sample data
    std::vector<std::vector<float>> data = {
        {1.0f, 2.0f, 3.0f, 4.0f},
        {2.0f, 1.0f, 2.0f, 3.0f},
        {3.0f, 2.0f, 1.0f, 2.0f},
        {4.0f, 3.0f, 2.0f, 1.0f},
        {10.0f, 10.0f, 10.0f, 10.0f}
    };

    // Add points to the index
    for (int i = 0; i < num_elements; i++) {
        appr_alg.addPoint(data[i].data(), i);
    }

    // Query vector
    std::vector<float> query = {1.5f, 2.0f, 3.0f, 4.0f};

    // Search for the 2 nearest neighbors
    auto result = appr_alg.searchKnn(query.data(), 2);

    std::cout << "Nearest neighbors:" << std::endl;
    while (!result.empty()) {
        auto &nn = result.top();
        // nn.first is the distance, nn.second is the label (index)
        std::cout << "Label: " << nn.second << " Distance: " << nn.first << std::endl;
        result.pop();
    }

    return 0;
}
