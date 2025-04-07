#ifndef BINARY_TREE_HPP
#define BINARY_TREE_HPP

#include <iostream>

/**
 * A node in a binary tree (C++ implementation)
 */
template <typename T>
struct TreeNode {
    T data;
    TreeNode<T>* left;
    TreeNode<T>* right;
    
    TreeNode(T value) : data(value), left(nullptr), right(nullptr) {}
};

/**
 * Binary search tree implementation in C++
 */
template <typename T>
class BinaryTree {
private:
    TreeNode<T>* root;
    
    // Helper functions
    void insert_recursive(TreeNode<T>*& node, T value);
    bool search_recursive(TreeNode<T>* node, T value);
    void destroy_recursive(TreeNode<T>* node);
    void inorder_recursive(TreeNode<T>* node);
    
public:
    BinaryTree() : root(nullptr) {}
    ~BinaryTree();
    
    void insert(T value);
    bool search(T value);
    void inorder_traversal();
    bool is_empty() const;
};

// Implementation of template methods
template <typename T>
BinaryTree<T>::~BinaryTree() {
    destroy_recursive(root);
}

template <typename T>
void BinaryTree<T>::destroy_recursive(TreeNode<T>* node) {
    if (node) {
        destroy_recursive(node->left);
        destroy_recursive(node->right);
        delete node;
    }
}

template <typename T>
void BinaryTree<T>::insert(T value) {
    insert_recursive(root, value);
}

template <typename T>
void BinaryTree<T>::insert_recursive(TreeNode<T>*& node, T value) {
    if (!node) {
        node = new TreeNode<T>(value);
    } else if (value < node->data) {
        insert_recursive(node->left, value);
    } else if (value > node->data) {
        insert_recursive(node->right, value);
    }
    // If value equals node->data, do nothing (no duplicates)
}

template <typename T>
bool BinaryTree<T>::search(T value) {
    return search_recursive(root, value);
}

template <typename T>
bool BinaryTree<T>::search_recursive(TreeNode<T>* node, T value) {
    if (!node) {
        return false;
    }
    if (node->data == value) {
        return true;
    }
    if (value < node->data) {
        return search_recursive(node->left, value);
    }
    return search_recursive(node->right, value);
}

template <typename T>
void BinaryTree<T>::inorder_traversal() {
    inorder_recursive(root);
    std::cout << std::endl;
}

template <typename T>
void BinaryTree<T>::inorder_recursive(TreeNode<T>* node) {
    if (node) {
        inorder_recursive(node->left);
        std::cout << node->data << " ";
        inorder_recursive(node->right);
    }
}

template <typename T>
bool BinaryTree<T>::is_empty() const {
    return root == nullptr;
}

#endif /* BINARY_TREE_HPP */