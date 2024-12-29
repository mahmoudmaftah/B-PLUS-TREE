# README: Advanced B+ Tree and ANN Index Project

## Table of Contents

1. [Problem Statement](#problem-statement)
2. [Directory Structure](#directory-structure)
3. [Implemented Components](#implemented-components)
    - [B+ Tree Implementations](#b-tree-implementations)
    - [Vector Index (HNSW + B+ Tree)](#vector-index-hnsw--b-tree)
    - [Naive Vector Index](#naive-vector-index)
    - [Test and Benchmarking Utilities](#test-and-benchmarking-utilities)
4. [Node Structure and Functions](#node-structure-and-functions)
5. [Optimizations and Design Decisions](#optimizations-and-design-decisions)
6. [How to Run and Benchmark](#how-to-run-and-benchmark)
7. [Future Extensions](#future-extensions)

---

## Problem Statement

This project addresses the problem of efficiently performing Approximate Nearest Neighbor (ANN) queries on large-scale vector datasets while simultaneously allowing scalar range filtering. ANN queries are fundamental in domains like recommendation systems, computer vision, and natural language processing.

The goal is to:

1. Efficiently store and retrieve vectors along with scalar metadata.
2. Combine scalar range filtering with ANN queries using a hybrid data structure.
3. Provide multiple implementations of B+ trees tailored for various use cases (unique keys, duplicate keys, and efficient range queries).
4. Create a benchmarking framework for performance evaluation and visualization.

---

## Directory Structure

The project is organized as follows:

```plaintext
hnswlib
├── include
│   ├── BPlusTree2.h             # B+ Tree implementation for unique keys.
│   ├── BPlusTree3.h             # B+ Tree with support for duplicate keys.
│   ├── BPlusTree4.h             # B+ Tree optimized for range queries.
│   ├── probabilisticVectorIndex.h  # Hybrid index (B+ Tree + HNSW).
│   ├── naiveVectorIndex.h       # Naive ANN index (linear scan for benchmarking).
│   ├── SequentialScan.h         # Simple sequential scan for validation.
│   ├── vectorIndex.h            # Generalized vector indexing interface.
├── src
│   # Implementation files (if required, optional for header-only classes).
├── tests
│   ├── _Data                    # Input data for testing and benchmarking.
│   │   ├── _data.csv            # Sample input vectors.
│   │   ├── _queries.csv         # Query vectors.
│   │   ├── key_value_pairs.txt  # Key-value pairs for B+ Tree testing.
│   ├── _Output                  # Benchmarking and result outputs.
│   │   ├── insertion_times      # Insertion time results for B+ Tree variants.
│   ├── Test2                    # Tests for specific components.
│   │   ├── Test2.cpp            # Unit tests for B+ Tree.
│   ├── Test3                    # Benchmarking for B+ Tree.
│   │   ├── insertionTimes.cpp   # Insertion time benchmarks.
│   ├── Test5                    # Tests for naive and optimized vector indices.
│   │   ├── naiveVectorIndexTest.cpp # Tests for naive vector index.
│   │   ├── vectorIndexTest.cpp  # Tests for hybrid vector index.
│   ├── Test6                    # Tests for probabilistic vector index.
│   │   ├── probabilisticVectorIndexTest.cpp # ANN + scalar filtering tests.
│   ├── generateData.py          # Python script for generating random data.
│   ├── generateVectorData.py    # Script for generating vector datasets.
```

---

## Implemented Components

### B+ Tree Implementations

We have implemented three versions of the B+ Tree, each tailored for specific use cases:

1. **Unique Key Mapping (BPlusTree2.h):**
   - Ensures each key maps to a single value.
   - Ideal for applications where duplicate keys are not allowed.

2. **Duplicate Keys Allowed (BPlusTree3.h):**
   - Supports multiple records with the same key.
   - Useful for scenarios like secondary indexing where multiple records share the same property.

3. **Optimized for Range Queries (BPlusTree4.h):**
   - Designed to handle efficient range queries with duplicate keys.
   - Essential for filtering operations in the hybrid vector index.

### Vector Index (HNSW + B+ Tree)

The **probabilisticVectorIndex.h** file combines the high-dimensional vector indexing capabilities of HNSW (Hierarchical Navigable Small World Graph) with the scalar filtering power of a B+ Tree. Key features include:

- **HNSW for ANN:** Provides efficient approximate nearest neighbor search.
- **B+ Tree for Filtering:** Filters results based on scalar metadata (e.g., a range [Smin, Smax]).
- **Probabilistic Candidate Selection:** Dynamically determines the number of candidates to fetch from HNSW to ensure high recall while minimizing unnecessary computations.

### Naive Vector Index

For benchmarking purposes, a naive vector index is implemented in **naiveVectorIndex.h**. This index performs linear scans over the entire dataset, ensuring correctness but without optimization. It serves as a baseline for evaluating the performance of the hybrid vector index.

### Test and Benchmarking Utilities

- **Data Generation:** Python scripts (
`generateData.py` and `generateVectorData.py`) generate random datasets or datasets following specific distributions (e.g., normal, uniform) to facilitate controlled benchmarking.
- **Test Framework:** Tests for all components are provided in the `tests` folder.
- **Output and Visualization:** Results (e.g., insertion times, query times) are saved to `_Output` and can be visualized using Python's plotting libraries.

---

## Interface description

### Node Structure
The core of the **ProbabilisticVectorIndex** class combines:

1. **B+ Tree Node:**
   - Key: Scalar value \(s\) (e.g., a numeric property).
   - Value: Index of the corresponding vector in the dataset.

2. **HNSW Node:**
   - Stores high-dimensional vectors.
   - Supports nearest neighbor search with approximate but highly efficient graph traversal.

The B+ Tree nodes allow efficient range queries, while the HNSW nodes enable fast approximate neighbor retrieval for vector data.

### Core Functions

#### Public Methods:
- **`ProbabilisticVectorIndex(int order):`**
  - Constructor to initialize the B+ Tree (with the specified order) and prepare the HNSW index.

- **`~ProbabilisticVectorIndex():`**
  - Destructor to clean up dynamically allocated memory for HNSW structures.

- **`void insert(const std::vector<float>& vec, float s):`**
  - Inserts a vector and its associated scalar value into the hybrid index.
  - Updates both the B+ Tree (for scalar filtering) and the HNSW index (for ANN queries).

- **`std::vector<int> query(const std::vector<float>& v, int k, float Smin, float Smax, double alpha = 0.01):`**
  - Performs a nearest neighbor search for the query vector `v`.
  - Filters candidates based on scalar range `[Smin, Smax]`.
  - Dynamically determines the number of candidates \(O\) to retrieve, ensuring a high probability of returning \(k\) valid results.

#### Private Methods:
- **`int computeRequiredO_BinarySearch(int M, int S, int k, double alpha):`**
  - Calculates the number of candidates \(O\) to fetch from HNSW using a binary search on the binomial cumulative distribution.

- **`double binomialCDFLessThan(int n, int k, double p):`**
  - Computes the probability \(P(X < k)\) for \(X \sim 	{Binomial}(n, p)\).

- **`std::vector<int> approximateNearestNeighbors(const std::vector<float>& query, int O):`**
  - Retrieves the top \(O\) approximate neighbors for a given query vector.

- **`float euclideanDistSquared(const std::vector<float>& a, const std::vector<float>& b):`**
  - Calculates the squared Euclidean distance between two vectors.

---

## Optimizations and Design Decisions

1. **Combination of B+ Tree and HNSW:**
   - The hybrid index exploits the strengths of both data structures: HNSW efficiently narrows down candidates based on proximity, while the B+ Tree filters these candidates by scalar constraints.
   
2. **Dynamic Candidate Selection:**
   - A probabilistic formula is used to determine the number of candidates (\(O\)) fetched from HNSW, ensuring that the probability of missing the required \(k\) nearest neighbors is negligible.

3. **Specialized B+ Trees:**
   - Different B+ Tree implementations are provided to cater to diverse requirements (unique keys, duplicate keys, and range queries).

4. **Benchmarking Tools:**
   - Naive solutions are implemented for benchmarking and validation.
   - Python scripts are used for visualization and in-depth analysis of results.

---

## How to Run and Benchmark

1. **Compile the Tests:**
   - Use your favorite C++ compiler (e.g., `g++` or `clang++`) to compile the test files. Example:
     ```bash
     g++ -std=c++17 -o test6 tests/Test6/probabilisticVectorIndexTest.cpp -I include
     ./test6
     ```

2. **Generate Data:**
   - Run the Python scripts to generate data for benchmarking:
     ```bash
     python3 tests/generateData.py
     ```

3. **Run Benchmarks:**
   - Run the specific test binaries (e.g., for insertion times or query times).
   - Outputs will be stored in the `_Output` folder for further analysis.


---

## Future Extensions

1. **Enhanced Probabilistic Model:**
   - Incorporate machine learning models to predict the number of candidates \(O\) based on historical query patterns.

2. **Distributed Indexing:**
   - Extend the hybrid index to support distributed storage and query execution.

3. **Additional Filtering Capabilities:**
   - Add support for more complex filtering conditions (e.g., multi-dimensional constraints).

4. **Dynamic Data Handling:**
   - Optimize the index for dynamic datasets where vectors and scalar values are updated frequently.

---

