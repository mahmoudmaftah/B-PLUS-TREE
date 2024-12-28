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

    // Public interface
    void insert(const KeyType& key, const ValueType& value);
    void remove(const KeyType& key);
    ValueType search(const KeyType& key) const;
    void traverse() const;

private:
    // Node structure
    struct Node {
        bool isLeaf;
        std::vector<KeyType> keys;
        std::vector<Node*> children;    // Used if internal node
        std::vector<ValueType> values;  // Used if leaf node
        Node* next;  // Pointer to next leaf node (used if leaf node)

        Node(bool leaf);
        ~Node();
    };

    // Root node of the B+ tree
    Node* root;

    // Order of the tree (maximum number of keys in a node)
    int order;

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




template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::BPlusTree(int order) : root(nullptr), order(order) {
    if (order < 3) {
        throw std::invalid_argument("Order must be at least 3");
    }
    // Create an empty root node
    root = new Node(true);
}

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::~BPlusTree() {
    delete root;
}

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::Node::Node(bool leaf) : isLeaf(leaf), next(nullptr) {
}

template <typename KeyType, typename ValueType>
BPlusTree<KeyType, ValueType>::Node::~Node() {
    if (!isLeaf) {
        for (auto child : children) {
            delete child;
        }
    }
}




template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::insert(const KeyType& key, const ValueType& value) {
    Node* leaf = root;

    // Traverse the tree to find the appropriate leaf node
    while (!leaf->isLeaf) {
        int i = std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key) - leaf->keys.begin();
        leaf = leaf->children[i];
    }

    // Insert the key and value into the leaf node
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = it - leaf->keys.begin();

    if (it != leaf->keys.end() && *it == key) {
        // Key already exists, update the value
        leaf->values[index] = value;
        return;
    }

    leaf->keys.insert(it, key);
    leaf->values.insert(leaf->values.begin() + index, value);

    // Check for overflow and split if necessary
    if (leaf->keys.size() >= order) {
        splitLeaf(leaf);
    }
}


template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::splitLeaf(Node* leaf) {
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
    // Find the position to insert the key
    auto it = std::upper_bound(current->keys.begin(), current->keys.end(), key);
    int index = it - current->keys.begin();

    // Insert the key and child pointer
    current->keys.insert(it, key);
    current->children.insert(current->children.begin() + index + 1, child);

    // Check for overflow and split if necessary
    if (current->keys.size() >= order) {
        splitInternal(current);
    }
}


template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::splitInternal(Node* internal) {
    int mid = internal->keys.size() / 2;

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
    Node* current = root;

    // Traverse the tree to find the leaf node
    while (!current->isLeaf) {
        int i = std::upper_bound(current->keys.begin(), current->keys.end(), key) - current->keys.begin();
        current = current->children[i];
    }

    // Search for the key in the leaf node
    auto it = std::lower_bound(current->keys.begin(), current->keys.end(), key);
    int index = it - current->keys.begin();

    if (it != current->keys.end() && *it == key) {
        // we will return an iterator to the value
        return current->values[index];
    } else {
        return ValueType();
    }
}




template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::traverse() const {
    Node* current = root;
    // Go to the leftmost leaf
    while (!current->isLeaf) {
        current = current->children.front();
    }

    // Traverse through the leaf nodes
    while (current != nullptr) {
        for (const auto& key : current->keys) {
            std::cout << key << " ";
        }
        current = current->next;
    }
    std::cout << std::endl;
}




template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::remove(const KeyType& key) {
    Node* leaf = root;

    // Traverse the tree to find the leaf node
    std::vector<Node*> path;
    while (!leaf->isLeaf) {
        path.push_back(leaf);
        int i = std::upper_bound(leaf->keys.begin(), leaf->keys.end(), key) - leaf->keys.begin();
        leaf = leaf->children[i];
    }

    path.push_back(leaf);

    // Find the key in the leaf node
    auto it = std::lower_bound(leaf->keys.begin(), leaf->keys.end(), key);
    int index = it - leaf->keys.begin();

    if (it == leaf->keys.end() || *it != key) {
        std::cout << "Key not found" << std::endl;
        return;
    }

    // Remove the key and value
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
    if (leaf->keys.size() < minKeys && leaf != root) {
        Node* parent = nullptr;
        int indexInParent = -1;

        // Find parent and index of leaf in parent's children
        for (int i = path.size() - 2; i >= 0; --i) {
            parent = path[i];
            for (size_t j = 0; j < parent->children.size(); ++j) {
                if (parent->children[j] == leaf) {
                    indexInParent = j;
                    break;
                }
            }
            if (indexInParent != -1) break;
        }

        // Try to borrow from left sibling
        if (indexInParent > 0) {
            Node* leftSibling = parent->children[indexInParent - 1];
            if (leftSibling->keys.size() > minKeys) {
                borrowFromLeftLeaf(leaf, leftSibling, parent, indexInParent);
                return;
            }
        }

        // Try to borrow from right sibling
        if (indexInParent < parent->children.size() - 1) {
            Node* rightSibling = parent->children[indexInParent + 1];
            if (rightSibling->keys.size() > minKeys) {
                borrowFromRightLeaf(leaf, rightSibling, parent, indexInParent);
                return;
            }
        }

        // Merge with sibling
        if (indexInParent > 0) {
            Node* leftSibling = parent->children[indexInParent - 1];
            mergeLeaf(leftSibling, leaf, parent, indexInParent - 1);
        } else if (indexInParent < parent->children.size() - 1) {
            Node* rightSibling = parent->children[indexInParent + 1];
            mergeLeaf(leaf, rightSibling, parent, indexInParent);
        }
    }
}




// For balancing the tree after removal

template <typename KeyType, typename ValueType>
void BPlusTree<KeyType, ValueType>::borrowFromLeftLeaf(Node* leaf, Node* leftSibling, Node* parent, int index) {
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
    } else if (parent->keys.size() < (order - 1) / 2) {
        // Implement further underflow handling for internal nodes
    }
}
