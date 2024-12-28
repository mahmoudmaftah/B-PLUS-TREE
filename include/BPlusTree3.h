#ifndef BPLUSTREE2_H
#define BPLUSTREE2_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <stdexcept>

template <typename KeyType, typename ValueType>
class BPlusTree {
public:
    BPlusTree(int order);
    ~BPlusTree();

    // Insert a (key, value) pair
    void insert(const KeyType& key, const ValueType& value);

    // Remove all values associated with a key
    void remove(const KeyType& key);

    // Returns the first value associated with the key (if any)
    ValueType search(const KeyType& key) const;

    // Returns a pointer to the vector of values associated with the key or nullptr if not found
    const std::vector<ValueType>* searchAll(const KeyType& key) const;

    // Traverse and print keys for debugging
    void traverse() const;

private:
    // Node structure
    struct Node {
        bool isLeaf;
        std::vector<KeyType> keys;
        std::vector<Node*> children; // Used if internal node
        // For leaf nodes, we store a vector of vectors of values:
        std::vector<std::vector<ValueType>> values;  // Used if leaf node
        Node* next;  // Pointer to next leaf node

        Node(bool leaf);
        ~Node();
    };

    Node* root;   // Root node of the B+ tree
    int order;     // Maximum number of keys in a node

    // Helper functions
    void insertInternal(const KeyType& key, Node* current, Node* child);
    void removeInternal(const KeyType& key, Node* current, Node* child);
    Node* findParent(Node* current, Node* child);

    // Utility functions for splitting and merging nodes
    void splitLeaf(Node* leaf);
    void splitInternal(Node* internal);
    void mergeLeaf(Node* left, Node* right, Node* parent, int index);
    void mergeInternal(Node* left, Node* right, Node* parent, int index);
    void borrowFromLeftLeaf(Node* leaf, Node* leftSibling, Node* parent, int index);
    void borrowFromRightLeaf(Node* leaf, Node* rightSibling, Node* parent, int index);
    void borrowFromLeftInternal(Node* node, Node* leftSibling, Node* parent, int index);
    void borrowFromRightInternal(Node* node, Node* rightSibling, Node* parent, int index);
};

#endif // BPLUSTREE2_H

// Implementation

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::BPlusTree(int order) : root(nullptr), order(order) {
    /**
     * @brief Constructor for the BPlusTree.
     * @param order The maximum number of keys a node can hold. Must be at least 3.
     * @throws std::invalid_argument if the order is less than 3.
     */
    if (order < 3) {
        throw std::invalid_argument("Order must be at least 3");
    }
    // Create an empty root node
    root = new Node(true);
}

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::~BPlusTree() {
    /**
     * @brief Destructor for the BPlusTree.
     */
    delete root;
}

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::Node::Node(bool leaf) : isLeaf(leaf), next(nullptr) {
    /**
     * @brief Constructor for a BPlusTree Node.
     * @param leaf Indicates whether the node is a leaf.
     */
}

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::Node::~Node() {
    /**
     * @brief Destructor for a BPlusTree Node.
     * Frees all dynamically allocated children if the node is an internal node.
     */
    if (!isLeaf) {
        for (auto child : children) {
            delete child;
        }
    }
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::insert(const KeyType& key, const ValueType& value) {
    /**
     * @brief Inserts a key-value pair into the BPlusTree.
     * @param key The key to be inserted.
     * @param value The value associated with the key.
     */

    Node* leaf = root;

    // Traverse the tree to find the appropriate leaf node O(log(n))
    while (!leaf->isLeaf) {
        int i = (int)(std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key) - leaf->keys.begin());
        leaf = leaf->children[i];   
    }

    // Insert the key and value into the leaf node
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = (int)(it - leaf->keys.begin());

    if (it != leaf->keys.end() && *it == key) {
        // Key already exists, append the value to the existing vector
        leaf->values[index].push_back(value);
    } else {
        // Insert new key and initialize values vector
        leaf->keys.insert(it, key);
        leaf->values.insert(leaf->values.begin() + index, std::vector<ValueType>{value});
    }

    // Check for overflow and split if necessary
    if ((int)leaf->keys.size() >= order) {
        splitLeaf(leaf);
    }
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::splitLeaf(Node* leaf) {
    /**
     * @brief Splits a leaf node into two when it exceeds the maximum allowed keys.
     * @param leaf The leaf node to be split.
     */
    int mid = (order + 1) / 2;

    // Create a new leaf node
    Node* newLeaf = new Node(true);
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    // Move half of the keys and values to the new leaf
    newLeaf->keys.assign(leaf->keys.begin() + mid, leaf->keys.end());
    newLeaf->values.assign(leaf->values.begin() + mid, leaf->values.end());

    leaf->keys.resize(mid);
    leaf->values.resize(mid);

    // Promote the first key of the new leaf to the parent
    KeyType newKey = newLeaf->keys.front();

    if (leaf == root) {
        // Create a new root node
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(newKey);
        newRoot->children.push_back(leaf);
        newRoot->children.push_back(newLeaf);
        root = newRoot;
    } else {
        // Insert the new key into the parent node
        Node* parent = findParent(root, leaf);
        insertInternal(newKey, parent, newLeaf);
    }
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::insertInternal(const KeyType& key, Node* current, Node* child) {
    /**
     * @brief Inserts a key and child pointer into an internal node.
     * @param key The key to be inserted.
     * @param current The internal node where the key will be inserted.
     * @param child The child pointer associated with the key.
     */

    // Find the position to insert the key
    auto it = std::upper_bound(current->keys.begin(), current->keys.end(), key);
    int index = (int)(it - current->keys.begin());

    // Insert the key and child pointer
    current->keys.insert(it, key);
    current->children.insert(current->children.begin() + index + 1, child);

    // Check for overflow and split if necessary
    if ((int)current->keys.size() >= order) {
        splitInternal(current);
    }
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::splitInternal(Node* internal) {
    /**
     * @brief Splits an internal node into two when it exceeds the maximum allowed keys.
     * @param internal The internal node to be split.
     */
    int mid = (int)internal->keys.size() / 2;

    // Create a new internal node
    Node* newInternal = new Node(false);

    // Promote the middle key
    KeyType upKey = internal->keys[mid];

    // Move half of the keys and children to the new internal node
    newInternal->keys.assign(internal->keys.begin() + mid + 1, internal->keys.end());
    newInternal->children.assign(internal->children.begin() + mid + 1, internal->children.end());

    // Resize the current internal node
    internal->keys.resize(mid);
    internal->children.resize(mid + 1);

    if (internal == root) {
        // Create a new root node
        Node* newRoot = new Node(false);
        newRoot->keys.push_back(upKey);
        newRoot->children.push_back(internal);
        newRoot->children.push_back(newInternal);
        root = newRoot;
    } else {
        // Insert the promoted key into the parent node
        Node* parent = findParent(root, internal);
        insertInternal(upKey, parent, newInternal);
    }
}

template <typename KeyType, typename ValueType>
typename BPlusTree<KeyType, ValueType>::Node* BPlusTree<KeyType, ValueType>::findParent(Node* current, Node* child) {
    /**
     * @brief Finds the parent of a given child node.
     * @param current The current node being inspected.
     * @param child The child node whose parent is being searched for.
     * @return The parent node if found, otherwise nullptr.
     */
    if (current->isLeaf || current->children.empty()) {
        return nullptr;
    }

    for (size_t i = 0; i < current->children.size(); ++i) {
        if (current->children[i] == child) {
            return current;
        } else {
            Node* parent = findParent(current->children[i], child);
            if (parent != nullptr) {
                return parent;
            }
        }
    }
    return nullptr;
}

template <typename KeyType, typename ValueType>
ValueType BPlusTree<KeyType, ValueType>::search(const KeyType& key) const {
    /**
     * @brief Searches for the first value associated with a given key.
     * @param key The key to search for.
     * @return The first value associated with the key, or a default-constructed value if not found.
     */

    Node* current = root;
    if (!current) return ValueType();

    // Traverse the tree to find the leaf node
    while (!current->isLeaf) {
        int i = (int)(std::upper_bound(current->keys.begin(), current->keys.end(), key) - current->keys.begin());
        current = current->children[i];
    }

    // Search for the key in the leaf node
    auto it = std::lower_bound(current->keys.begin(), current->keys.end(), key);
    int index = (int)(it - current->keys.begin());

    if (it != current->keys.end() && *it == key) {
        // Return the first value associated with this key
        return current->values[index].empty() ? ValueType() : current->values[index].front();
    } else {
        return ValueType();
    }
}

template <typename KeyType, typename ValueType>
const std::vector<ValueType>* BPlusTree<KeyType, ValueType>::searchAll(const KeyType& key) const {
    /**
     * @brief Searches for all values associated with a given key.
     * @param key The key to search for.
     * @return A pointer to a vector of values if the key is found, or nullptr if not found.
     */

    Node* current = root;
    if (!current) return nullptr;

    // Traverse the tree to find the leaf node
    while (!current->isLeaf) {
        int i = (int)(std::upper_bound(current->keys.begin(), current->keys.end(), key) - current->keys.begin());
        current = current->children[i];
    }

    // Search for the key in the leaf node
    auto it = std::lower_bound(current->keys.begin(), current->keys.end(), key);
    int index = (int)(it - current->keys.begin());

    if (it != current->keys.end() && *it == key) {
        return &current->values[index];
    } else {
        return nullptr;
    }
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::traverse() const {
    /**
     * @brief Traverses the BPlusTree and prints all keys in the leaf nodes for debugging purposes.
     */

    if (!root) return;
    Node* current = root;
    // Go to the leftmost leaf
    while (!current->isLeaf) {
        current = current->children.front();
    }

    // Traverse through the leaf nodes
    while (current != nullptr) {
        for (size_t i = 0; i < current->keys.size(); ++i) {
            std::cout << current->keys[i] << ":[";
            for (size_t j = 0; j < current->values[i].size(); j++) {
                std::cout << current->values[i][j];
                if (j+1 < current->values[i].size()) std::cout << ", ";
            }
            std::cout << "] ";
        }
        current = current->next;
    }
    std::cout << std::endl;
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::remove(const KeyType& key) {
    /**
     * @brief Removes all values associated with a given key from the BPlusTree.
     * Handles underflow conditions by borrowing or merging nodes if necessary.
     * @param key The key to be removed.
     */

    if (!root) return;

    Node* leaf = root;
    // Traverse the tree to find the leaf node
    std::vector<Node*> path;
    while (!leaf->isLeaf) {
        path.push_back(leaf);
        int i = (int)(std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key) - leaf->keys.begin());
        leaf = leaf->children[i];
    }

    path.push_back(leaf);

    // Find the key in the leaf node
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = (int)(it - leaf->keys.begin());

    if (it == leaf->keys.end() || *it != key) {
        std::cout << "Key not found" << std::endl;
        return;
    }

    // Remove the key and all its associated values
    leaf->keys.erase(it);
    leaf->values.erase(leaf->values.begin() + index);

    // If the leaf node is the root, and it's empty, update the root
    if (leaf == root && leaf->keys.empty()) {
        delete root;
        root = nullptr;
        return;
    }

    // Handle underflow
    int minKeys = (order - 1) / 2;
    if ((int)leaf->keys.size() < minKeys && leaf != root) {
        Node* parent = nullptr;
        int indexInParent = -1;

        // Find parent and index of leaf in parent's children
        for (int i = (int)path.size() - 2; i >= 0; --i) {
            parent = path[i];
            for (size_t j = 0; j < parent->children.size(); ++j) {
                if (parent->children[j] == leaf) {
                    indexInParent = (int)j;
                    break;
                }
            }
            if (indexInParent != -1) break;
        }

        // Try to borrow from left sibling
        if (indexInParent > 0) {
            Node* leftSibling = parent->children[indexInParent - 1];
            if ((int)leftSibling->keys.size() > minKeys) {
                borrowFromLeftLeaf(leaf, leftSibling, parent, indexInParent);
                return;
            }
        }

        // Try to borrow from right sibling
        if (indexInParent < (int)parent->children.size() - 1) {
            Node* rightSibling = parent->children[indexInParent + 1];
            if ((int)rightSibling->keys.size() > minKeys) {
                borrowFromRightLeaf(leaf, rightSibling, parent, indexInParent);
                return;
            }
        }

        // Merge with sibling
        if (indexInParent > 0) {
            Node* leftSibling = parent->children[indexInParent - 1];
            mergeLeaf(leftSibling, leaf, parent, indexInParent - 1);
        } else if (indexInParent < (int)parent->children.size() - 1) {
            Node* rightSibling = parent->children[indexInParent + 1];
            mergeLeaf(leaf, rightSibling, parent, indexInParent);
        }
    }
}

// For balancing the tree after removal
template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::borrowFromLeftLeaf(Node* leaf, Node* leftSibling, Node* parent, int index) {
    /**
     * @brief Borrows a key-value pair from the left sibling of a leaf node to balance the tree.
     * @param leaf The underflowing leaf node.
     * @param leftSibling The left sibling of the leaf node.
     * @param parent The parent node of the leaf and its sibling.
     * @param index The index of the leaf in the parent's children.
     */

    // Move the last key-value pair from left sibling to the front of leaf
    leaf->keys.insert(leaf->keys.begin(), leftSibling->keys.back());
    leaf->values.insert(leaf->values.begin(), leftSibling->values.back());
    leftSibling->keys.pop_back();
    leftSibling->values.pop_back();

    // Update parent key
    parent->keys[index - 1] = leaf->keys.front();
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::borrowFromRightLeaf(Node* leaf, Node* rightSibling, Node* parent, int index) {
    /**
     * @brief Borrows a key-value pair from the right sibling of a leaf node to balance the tree.
     * @param leaf The underflowing leaf node.
     * @param rightSibling The right sibling of the leaf node.
     * @param parent The parent node of the leaf and its sibling.
     * @param index The index of the leaf in the parent's children.
     */

    // Move the first key-value pair from right sibling to the end of leaf
    leaf->keys.push_back(rightSibling->keys.front());
    leaf->values.push_back(rightSibling->values.front());
    rightSibling->keys.erase(rightSibling->keys.begin());
    rightSibling->values.erase(rightSibling->values.begin());

    // Update parent key
    parent->keys[index] = rightSibling->keys.front();
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::mergeLeaf(Node* left, Node* right, Node* parent, int index) {
    /**
     * @brief Merges a leaf node with its sibling when it underflows.
     * @param left The left leaf node.
     * @param right The right leaf node.
     * @param parent The parent node of the two leaf nodes.
     * @param index The index of the right leaf in the parent's children.
     */

    // Move all keys and values from right to left
    left->keys.insert(left->keys.end(), right->keys.begin(), right->keys.end());
    left->values.insert(left->values.end(), right->values.begin(), right->values.end());
    left->next = right->next;

    // Remove right sibling
    parent->keys.erase(parent->keys.begin() + index);
    parent->children.erase(parent->children.begin() + index + 1);
    delete right;

    // Handle parent underflow
    if (parent == root && parent->keys.empty()) {
        root = left;
        delete parent;
    } else if ((int)parent->keys.size() < (order - 1) / 2) {
        // Implement further underflow handling for internal nodes if needed
    }
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::mergeInternal(Node* left, Node* right, Node* parent, int index) {
    /**
     * @brief Merges two internal nodes if one of them underflows.
     * @param left The left internal node.
     * @param right The right internal node.
     * @param parent The parent node of the two internal nodes.
     * @param index The index of the right internal node in the parent's children.
     */
    // Implement if needed for internal node merging
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::borrowFromLeftInternal(Node* node, Node* leftSibling, Node* parent, int index) {
    /**
     * @brief Balances the tree by borrowing a key from the left sibling of an internal node.
     * @param node The underflowing internal node.
     * @param leftSibling The left sibling of the internal node.
     * @param parent The parent node of the internal node and its sibling.
     * @param index The index of the internal node in the parent's children.
     */
    // Implement if needed for internal node balancing
}

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::borrowFromRightInternal(Node* node, Node* rightSibling, Node* parent, int index) {
    /**
     * @brief Balances the tree by borrowing a key from the right sibling of an internal node.
     * @param node The underflowing internal node.
     * @param rightSibling The right sibling of the internal node.
     * @param parent The parent node of the internal node and its sibling.
     * @param index The index of the internal node in the parent's children.
     */
    // Implement if needed for internal node balancing
}

