import random
import string

def generate_key_value_pairs(num_pairs, key_length=10, value_range=(1, 100000)):
    """
    Generates a dictionary of random key-value pairs.

    Args:
        num_pairs (int): The number of key-value pairs to generate.
        key_length (int): The length of each randomly generated key. Default is 10.
        value_range (tuple): A tuple specifying the range (inclusive) of the random values. Default is (1, 100000).

    Returns:
        dict: A dictionary where each key is a random alphanumeric string and each value is a random integer within the specified range.
    """
    key_value_pairs = {}

    # Generate the specified number of key-value pairs
    for _ in range(num_pairs):
        # Generate a random key of the specified length using letters and digits
        key = ''.join(random.choices(string.ascii_letters + string.digits, k=key_length))

        # Generate a random value within the specified range
        value = random.randint(*value_range)

        # Add the key-value pair to the dictionary
        key_value_pairs[key] = value

    return key_value_pairs

if __name__ == "__main__":
    # Specify the number of key-value pairs to generate
    num_pairs = 100000

    # Generate the key-value pairs
    key_value_pairs = generate_key_value_pairs(num_pairs)

    # Save the key-value pairs to a file
    with open("key_value_pairs.txt", "w") as f:
        for key, value in key_value_pairs.items():
            # Write each key-value pair in the format: key value
            f.write(f"{key} {value}\n")

    # Print a message indicating successful generation and saving
    print(f"Generated {num_pairs} key-value pairs and saved to 'key_value_pairs.txt'.")
