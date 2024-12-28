import random
import math
import os
from scipy.stats import norm

def generate_data(N=10000, D=4,
                 vec_range=(0.0, 10.0),
                 s_mean=50.0, s_variance=25.0,
                 p=0.01,  # Desired proportion of items satisfying [Smin, Smax]
                 k_range=(1, 10), O_value=1000):
    """
    Generates data and a single query for testing the VectorIndex.

    Args:
        N (int): Number of data points to generate. Default is 10000.
        D (int): Dimension of vectors. Default is 4.
        vec_range (tuple): Range for vector values. Default is (0.0, 10.0).
        s_mean (float): Mean of the normal distribution for scalar values. Default is 50.0.
        s_variance (float): Variance of the normal distribution for scalar values. Default is 25.0.
        p (float): Desired proportion of items to satisfy the scalar condition [Smin, Smax]. Must be between 0 and 1. Default is 0.01.
        k_range (tuple): Range for k values in the query. Default is (1, 10).
        O_value (int): Fixed O value for the query. Default is 1000.

    Writes:
        ./tests/_Data/_data.csv: Generated data with vector and scalar values.
        ./tests/_Data/_queries.csv: Generated query with vector, k, Smin, Smax, and O values.
    """
    # Validate proportion p
    if not (0 < p < 1):
        raise ValueError("Parameter 'p' must be between 0 and 1 (exclusive).")

    # Ensure the output directory exists
    output_dir = "./tests/_Data/"
    os.makedirs(output_dir, exist_ok=True)

    # File paths
    data_file = os.path.join(output_dir, "_data3.csv")
    queries_file = os.path.join(output_dir, "_queries3.csv")

    # Write data to _data.csv
    with open(data_file, "w") as f_data:
        # Header for data
        data_header = [f"v{i+1}" for i in range(D)] + ["s"]
        f_data.write(",".join(data_header) + "\n")

        for _ in range(N):
            # Generate a random vector
            vec = [random.uniform(*vec_range) for _ in range(D)]
            # Generate a scalar value from a normal distribution
            s = random.gauss(s_mean, math.sqrt(s_variance))
            # Write the data point
            line = ",".join(map(str, vec)) + "," + f"{s:.5f}" + "\n"
            f_data.write(line)

    # Compute Smin and Smax based on desired proportion p
    # Using two-tailed interval around the mean
    z = norm.ppf((1 + p) / 2)  # z-score for the two-tailed p
    Smin = s_mean - z * math.sqrt(s_variance)
    Smax = s_mean + z * math.sqrt(s_variance)

    # Write query to _queries.csv
    with open(queries_file, "w") as f_queries:
        # Header for queries
        query_header = [f"qv{i+1}" for i in range(D)] + ["k", "Smin", "Smax", "O"]
        f_queries.write(",".join(query_header) + "\n")

        # Generate a single query vector
        qvec = [random.uniform(*vec_range) for _ in range(D)]
        # Randomly choose k from k_range
        k = random.randint(*k_range)

        # Format Smin and Smax to 5 decimal places
        Smin_formatted = f"{Smin:.5f}"
        Smax_formatted = f"{Smax:.5f}"

        # Write the query
        query_line = ",".join(map(str, qvec)) + f",{k},{Smin_formatted},{Smax_formatted},{O_value}\n"
        f_queries.write(query_line)

    print(f"Data generation complete. {N} data points written to {data_file}.")
    print(f"Single query written to {queries_file} with [Smin, Smax] = [{Smin_formatted}, {Smax_formatted}].")

if __name__ == "__main__":
    # Default parameters
    N = 10000          # Number of data points
    D = 4              # Dimension of vectors
    a = 50.0           # Mean of the normal distribution for s
    b = 25.0           # Variance of the normal distribution for s
    p = 0.01            # Desired proportion of items satisfying the query condition
    k_range = (10, 10)  # Range for k values in the query
    O_value = 1000     # Fixed O value for the query

    # Generate data and a single query
    generate_data(N=N, D=D, s_mean=a, s_variance=b, p=p, k_range=k_range, O_value=O_value)
