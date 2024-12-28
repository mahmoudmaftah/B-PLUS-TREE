import random
import sys
import os

def generate_data(N=10000, D=4, Q=10,
                 vec_range=(0.0, 10.0), s_range=(0.0, 100.0),
                 query_s_width=0.1,  # Width of the scalar range for queries
                 k_range=(1, 10), O_value=1000):
    """
    Generates data and query files for testing the VectorIndex.
    Ensures that each query's scalar range [Smin, Smax] is narrow,
    resulting in only a few records passing the filtering test.

    Args:
        N (int): Number of data points to generate. Default is 10000.
        D (int): Dimension of vectors. Default is 4.
        Q (int): Number of queries to generate. Default is 10.
        vec_range (tuple): Range for vector values. Default is (0.0, 10.0).
        s_range (tuple): Range for scalar values in data. Default is (0.0, 100.0).
        query_s_width (float): Width of the scalar range for each query.
                                Smaller values result in fewer matching records.
                                Default is 0.1.
        k_range (tuple): Range for k values in queries. Default is (1, 10).
        O_value (int): Fixed O value for queries. Default is 1000.

    Writes:
        ./tests/_Data/_data.csv: Generated data with vector and scalar values.
        ./tests/_Data/_queries.csv: Generated queries with vector, k, Smin, Smax, and O values.
    """
    # Ensure the output directory exists
    os.makedirs("./tests/_Data/", exist_ok=True)

    # Write data to _data.csv
    with open("./tests/_Data/_data2.csv", "w") as f_data:
        # Header for data
        data_header = [f"v{i+1}" for i in range(D)] + ["s"]
        f_data.write(",".join(data_header) + "\n")

        for i in range(N):
            # Generate a random vector and scalar value
            vec = [random.uniform(*vec_range) for _ in range(D)]
            s = random.uniform(*s_range)
            line = ",".join(map(str, vec)) + "," + str(s) + "\n"
            f_data.write(line)

    # Write queries to _queries.csv
    with open("./tests/_Data/_queries2.csv", "w") as f_queries:
        # Header for queries
        query_header = [f"qv{i+1}" for i in range(D)] + ["k", "Smin", "Smax", "O"]
        f_queries.write(",".join(query_header) + "\n")

        for i in range(Q):
            # Generate a random query vector
            qvec = [random.uniform(*vec_range) for _ in range(D)]
            k = random.randint(*k_range)

            # Generate Smin ensuring that Smin + query_s_width does not exceed s_range[1]
            Smin_lower = s_range[0]
            Smin_upper = s_range[1] - query_s_width
            Smin = random.uniform(Smin_lower, Smin_upper)
            Smax = Smin + query_s_width

            # Optional: Introduce slight randomness to ensure some variation
            # For example, jitter Smin and Smax slightly
            jitter = query_s_width * 0.05  # 5% of the query width
            Smin = max(s_range[0], Smin - random.uniform(-jitter, jitter))
            Smax = min(s_range[1], Smax + random.uniform(-jitter, jitter))

            # Ensure Smin <= Smax after jitter
            if Smin > Smax:
                Smin, Smax = Smax, Smin

            line = ",".join(map(str, qvec)) + f",{k},{Smin},{Smax},{O_value}\n"
            f_queries.write(line)

if __name__ == "__main__":
    # Default values
    N = 10000      # Number of data points
    D = 4          # Dimension of vectors
    Q = 10         # Number of queries

    # Generate data and queries
    generate_data(N, D, Q)
