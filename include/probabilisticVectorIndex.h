#ifndef PROBABILISTIC_VECTOR_INDEX_H
#define PROBABILISTIC_VECTOR_INDEX_H

#include <cmath>
#include <vector>
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>

// Include your B+ tree header (as before)
#include "./bplustree4.h"

// HNSW library
#include "../hnswlib/hnswlib/hnswlib.h"

/**
 * @brief A vector index that combines a B+ Tree and HNSW, 
 *        using a probabilistic approach to pick the candidate size (O).
 */
class ProbabilisticVectorIndex {
public:
    /**
     * @brief Constructor.
     * @param order B+ tree order (minimum number of children per internal node).
     */
    ProbabilisticVectorIndex(int order)
        : tree(order), dimension(0), hnswIndex(nullptr), space(nullptr),
          hnswM(16), hnswEfConstruction(200), hnswEfSearch(200)
    {}

    /**
     * @brief Destructor: cleans up HNSW index and space, if created.
     */
    ~ProbabilisticVectorIndex() {
        delete hnswIndex;
        delete space;
    }

    /**
     * @brief Inserts a vector and its scalar value s into both the B+ Tree and the HNSW index.
     * @param vec The data vector (feature vector).
     * @param s   The scalar value used for filtering (e.g., some property).
     * @throws std::invalid_argument if the vector is empty or dimension mismatches existing data.
     */
    void insert(const std::vector<float>& vec, float s) {
        if (vec.empty()) {
            throw std::invalid_argument("Cannot insert empty vector");
        }

        // Initialize dimension and HNSW structures if this is the first insert
        if (dataVectors.empty()) {
            dimension = static_cast<int>(vec.size());

            // Create an L2 space and an HNSW index with a max capacity
            size_t maxElements = 100000; // You can adjust this based on expected dataset size
            space = new hnswlib::L2Space(dimension);
            hnswIndex = new hnswlib::HierarchicalNSW<float>(space, maxElements, hnswM, hnswEfConstruction);

            // Set the search ef parameter
            hnswIndex->setEf(hnswEfSearch);
        } else {
            // Ensure dimension consistency
            if (static_cast<int>(vec.size()) != dimension) {
                throw std::invalid_argument("All vectors must have the same dimension");
            }
        }

        // Index assignment
        int idx = static_cast<int>(dataVectors.size());
        dataVectors.push_back(vec);
        sValues.push_back(s);

        // Insert into B+ Tree (key = s, value = idx)
        tree.insert(s, idx);

        // Insert vector into HNSW
        hnswIndex->addPoint(dataVectors[idx].data(), idx);
    }

    /**
     * @brief Performs a k-NN query for the vector @p v while filtering by s in [Smin, Smax].
     *        Uses a probabilistic formula to pick the candidate size O for HNSW.
     * @param v      The query vector.
     * @param k      The number of neighbors to return.
     * @param Smin   The lower bound of the scalar filter.
     * @param Smax   The upper bound of the scalar filter.
     * @param alpha  Confidence parameter (probability of missing >= k matches is <= alpha).
     * @return A vector of up to k indices of nearest neighbors satisfying the scalar filter.
     * @throws std::invalid_argument if the dimension of v does not match the index dimension.
     */
    std::vector<int> query(const std::vector<float>& v, int k,
                           float Smin, float Smax, double alpha = 0.01)
    {
        // Sanity checks
        if (hnswIndex == nullptr || dataVectors.empty()) {
            return {}; // no data
        }
        if (static_cast<int>(v.size()) != dimension) {
            throw std::invalid_argument("Query vector dimension does not match index dimension");
        }

        // Count how many data points satisfy [Smin, Smax]
        int S = tree.countInRange(Smin, Smax); // number of valid points
        std::cout << "S: " << S << std::endl;
        if (S <= 0) {
            return {}; // none satisfy the condition
        }

        // Next, let M = total number of data points.
        int M = static_cast<int>(dataVectors.size());

        // We will choose O using our new "enhanced" method
        int O = computeRequiredO_Enhanced(M, S, k, alpha);  
        std::cout << "Chosen O: " << O << std::endl;

        // 1) Retrieve O approximate neighbors from HNSW
        //    Make sure efSearch is at least O so we actually can retrieve that many
        hnswIndex->setEf(std::max(hnswEfSearch, O + 50));  // ensure enough exploration
        std::vector<int> annCandidates = approximateNearestNeighbors(v, O);

        // 2) Filter them by checking if sValues[idx] is in [Smin, Smax]
        std::vector<std::pair<float,int>> distIndex;
        distIndex.reserve(annCandidates.size());
        for (int idx : annCandidates) {
            float sVal = sOfIndex(idx);
            if (sVal >= Smin && sVal <= Smax) {
                float dist = euclideanDistSquared(v, dataVectors[idx]);
                distIndex.push_back({dist, idx});
            }
        }

        // 3) Sort by distance ascending
        std::sort(distIndex.begin(), distIndex.end(),
                  [](auto& a, auto& b) { return a.first < b.first; });

        // 4) Return top-k
        std::vector<int> result;
        result.reserve(k);
        for (int i = 0; i < k && i < static_cast<int>(distIndex.size()); i++) {
            result.push_back(distIndex[i].second);
        }
        return result;
    }

private:
    // -- B+ Tree for scalar queries --
    BPlusTree<float, int> tree;

    // -- In-memory storage for vectors and their scalar filter values --
    std::vector<std::vector<float>> dataVectors;
    std::vector<float> sValues;
    int dimension; // dimension of all vectors

    // -- HNSW components --
    hnswlib::HierarchicalNSW<float>* hnswIndex;
    hnswlib::SpaceInterface<float>* space;
    int hnswM;
    int hnswEfConstruction;
    int hnswEfSearch;

    /**
     * @brief Gets the scalar s-value for index idx.
     */
    inline float sOfIndex(int idx) const {
        return sValues[idx];
    }

    /**
     * @brief Computes squared Euclidean distance between two vectors of the same dimension.
     */
    inline float euclideanDistSquared(const std::vector<float>& a, const std::vector<float>& b) const {
        float dist = 0.0f;
        for (size_t i = 0; i < a.size(); i++) {
            float diff = a[i] - b[i];
            dist += diff * diff;
        }
        return dist;
    }

    /**
     * @brief Approximate Nearest Neighbors from HNSW. Retrieves the top O candidates.
     * @param query The query vector.
     * @param O     The number of candidates to retrieve.
     * @return A vector of indices for the top O approximate neighbors.
     */
    std::vector<int> approximateNearestNeighbors(const std::vector<float>& query, int O) {
        if (!hnswIndex || O <= 0) {
            return {};
        }
        auto result = hnswIndex->searchKnn(query.data(), O);

        // hnswlib returns a priority queue (max at top), so we collect in reverse
        std::vector<int> candidates;
        candidates.reserve(result.size());
        while (!result.empty()) {
            candidates.push_back(static_cast<int>(result.top().second));
            result.pop();
        }
        // The queue gives them in reverse order, so we flip it
        std::reverse(candidates.begin(), candidates.end());
        return candidates;
    }

    // ----------------------------------------------------------------
    //           BINOMIAL/PROBABILISTIC APPROACH FOR SELECTING O
    // ----------------------------------------------------------------

    /**
     * @brief Computes the binomial coefficient C(n, k) = n! / (k! * (n-k)!).
     *        Uses a safe floating-point approach to reduce overflow.
     */
    double binomialCoefficient(int n, int k) const {
        if (k > n) return 0.0;
        if (k == 0 || k == n) return 1.0;
        double result = 1.0;
        // For efficiency, do k = min(k, n-k)
        int kk = (k < (n - k)) ? k : (n - k);
        for (int i = 1; i <= kk; i++) {
            result = result * (n - (kk - i)) / i;
        }
        return result;
    }

    /**
     * @brief Binomial PMF: P(X = k) for X ~ Binomial(n, p).
     */
    double binomialPMF(int n, int k, double p) const {
        if (p < 0.0 || p > 1.0) return 0.0;
        double c = binomialCoefficient(n, k);
        double pk  = std::pow(p,   k);
        double pmk = std::pow(1.0 - p, n - k);
        return c * pk * pmk;
    }

    /**
     * @brief Probability that a Binomial(n, p) random variable is less than k.
     *        i.e. P(X < k).
     */
    double binomialCDFLessThan(int n, int k, double p) const {
        double sumProb = 0.0;
        for (int i = 0; i < k; i++) {
            sumProb += binomialPMF(n, i, p);
        }
        return sumProb;
    }

    // ----------------------------------------------------------------
    //       ENHANCED METHODS FOR COMPUTING THE REQUIRED O
    // ----------------------------------------------------------------

    /**
     * @brief A direct (linear) approach as in your original code:
     *        increments O until P(X < k) <= alpha or O reaches M.
     *        (Kept here for reference; we’ll prefer the binary search version below.)
     */
    int computeRequiredO_Linear(int M, int S, int k, double alpha) const {
        if (k <= 0) return 0;
        if (S <= 0) return k;
        if (S >= M) return k;
        if (alpha <= 0.0) return k;

        double p = static_cast<double>(S) / M;
        int O = k; 
        while (O <= M) {
            double probFewerThanK = binomialCDFLessThan(O, k, p);
            if (probFewerThanK <= alpha) {
                return O;
            }
            O++;
        }
        return M; 
    }

    /**
     * @brief A faster binary-search-based approach to find the smallest O 
     *        so that P(X < k) <= alpha for X ~ Binomial(O, p).
     */
    int computeRequiredO_BinarySearch(int M, int S, int k, double alpha) const {
        // Edge cases
        if (k <= 0) return 0;
        if (S <= 0) return k;
        if (S >= M) return k;
        if (alpha <= 0.0) return k;

        double p = static_cast<double>(S) / M;
        int left = k;
        int right = M;
        int best = M;

        while (left <= right) {
            int mid = (left + right) / 2;
            double probFewerThanK = binomialCDFLessThan(mid, k, p);

            if (probFewerThanK <= alpha) {
                best = mid;      // mid is good enough
                right = mid - 1; // try to find a smaller O
            } else {
                left = mid + 1;  // need bigger O
            }
        }
        return best;
    }

    /*  OPTIONAL: Normal approximation approach for large M, S, O
    //--------------------------------------------------------------------
    //  If you prefer to approximate Binomial(O, p) by Normal(Op, Op(1-p))
    //  and solve for P(X < k) <= alpha, you can use this. Then in
    //  computeRequiredO_Enhanced() you can call computeRequiredO_NormalApprox
    //  instead of computeRequiredO_BinarySearch.
    //--------------------------------------------------------------------
    
    int computeRequiredO_NormalApprox(int M, int S, int k, double alpha) const {
        if (k <= 0) return 0;
        if (S <= 0) return k;
        if (S >= M) return k;
        if (alpha <= 0.0) return k;

        double p = static_cast<double>(S) / M;
        int left = k;
        int right = M;
        int best = M;

        // We convert alpha -> zAlpha for standard normal
        // e.g. alpha=0.01 => zAlpha ~ -2.33 (but we want 'X < k')
        // We'll define a simple function to get approximate zAlpha
        double zAlpha = invStdNormal(alpha);

        while (left <= right) {
            int mid = (left + right) / 2;
            double mean  = mid * p;
            double var   = mid * p * (1.0 - p);
            double stdev = std::sqrt(var + 1e-12);

            // continuity correction: check if (k - 0.5 - mean)/stdev <= zAlpha
            double lhs = (k - 0.5 - mean) / stdev;

            if (lhs <= zAlpha) {
                best = mid;
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }
        return best;
    }
    */

    /**
     * @brief A small approximate inverse CDF for the standard normal distribution.
     *        (You can replace this with a more accurate routine or use a library.)
     */
    double invStdNormal(double p) const {
        // For p in (0,1), we find z so that Phi(z) = p
        // This approximation (by Peter J. Acklam) works reasonably well.
        // If you want a simpler approach, you can just do something basic or call a library.
        
        if (p <= 0.0)  return -1e10;
        if (p >= 1.0)  return  1e10;

        static const double a1 = -3.969683028665376e+01;
        static const double a2 =  2.209460984245205e+02;
        static const double a3 = -2.759285104469687e+02;
        static const double a4 =  1.383577518672690e+02;
        static const double a5 = -3.066479806614716e+01;
        static const double a6 =  2.506628277459239e+00;

        static const double b1 = -5.447609879822406e+01;
        static const double b2 =  1.615858368580409e+02;
        static const double b3 = -1.556989798598866e+02;
        static const double b4 =  6.680131188771972e+01;
        static const double b5 = -1.328068155288572e+01;

        // constants
        static const double c1 = -7.784894002430293e-03;
        static const double c2 = -3.223964580411365e-01;
        static const double c3 = -2.400758277161838e+00;
        static const double c4 = -2.549732539343734e+00;
        static const double c5 =  4.374664141464968e+00;
        static const double c6 =  2.938163982698783e+00;

        static const double d1 =  7.784695709041462e-03;
        static const double d2 =  3.224671290700398e-01;
        static const double d3 =  2.445134137142996e+00;
        static const double d4 =  3.754408661907416e+00;

        // Define break-points
        double p_low  = 0.02425;
        double p_high = 1.0 - p_low;

        // rational approximation for lower region
        if (p < p_low) {
            double q  = std::sqrt(-2.0 * std::log(p));
            return (((((c1*q + c2)*q + c3)*q + c4)*q + c5)*q + c6) /
                   ((((d1*q + d2)*q + d3)*q + d4)*q + 1.0);
        }
        // rational approximation for upper region
        if (p > p_high) {
            double q  = std::sqrt(-2.0 * std::log(1.0 - p));
            return -(((((c1*q + c2)*q + c3)*q + c4)*q + c5)*q + c6) /
                     ((((d1*q + d2)*q + d3)*q + d4)*q + 1.0);
        }
        // rational approximation for central region
        double q = p - 0.5;
        double r = q * q;
        return (((((a1*r + a2)*r + a3)*r + a4)*r + a5)*r + a6)*q /
               (((((b1*r + b2)*r + b3)*r + b4)*r + b5)*r + 1.0);
    }

    /**
     * @brief Final "enhanced" method that uses binary search (or normal approx) + margin.
     */
    int computeRequiredO_Enhanced(int M, int S, int k, double alpha) const {
        // Use the binary-search-based method
        int O = computeRequiredO_BinarySearch(M, S, k, alpha);

        // Add a safety margin
        O += 100;
        return O;
    }
};

#endif // PROBABILISTIC_VECTOR_INDEX_H
