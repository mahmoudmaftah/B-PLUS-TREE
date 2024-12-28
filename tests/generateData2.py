import random
import string

def generate_random(num_pairs, value_length=10, key_range=(1, 1000000)):
    """
    Generates a list of random key-value pairs.

    Args:
        num_pairs (int): The number of key-value pairs to generate.
        value_length (int): The length of each randomly generated value. Default is 10.
        key_range (tuple): A tuple specifying the range (inclusive) of the random keys. Default is (1, 1000000).

    Returns:
        list: A list of tuples, where each tuple contains a random integer key and a random alphanumeric string value.
    """
    key_value_pairs = []

    # Generate the specified number of key-value pairs
    for _ in range(num_pairs):
        # Generate a random key within the specified range
        key = random.randint(*key_range)

        # Generate a random value of the specified length using letters and digits
        value = ''.join(random.choices(string.ascii_letters + string.digits, k=value_length))

        # Append the key-value pair as a tuple to the list
        key_value_pairs.append((key, value))

    return key_value_pairs

if __name__ == "__main__":
    # Specify the number of key-value pairs to generate
    num_pairs = 100000

    # Generate the key-value pairs
    key_value_pairs = generate_random(num_pairs)

    # Save the key-value pairs to a file
    with open("key_value_pairs_2.txt", "w") as f:
        for key, value in key_value_pairs:
            # Write each key-value pair in the format: key value
            f.write(f"{key} {value}\n")

    # Print a message indicating successful generation and saving
    print(f"Generated {num_pairs} key-value pairs and saved to 'key_value_pairs_2.txt'.")
