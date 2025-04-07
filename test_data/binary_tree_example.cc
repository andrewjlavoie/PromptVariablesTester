#include "binary_tree.hpp"
#include <iostream>

int main() {
    // Create a binary tree of integers
    BinaryTree<int> intTree;
    
    // Insert some values
    intTree.insert(50);
    intTree.insert(30);
    intTree.insert(70);
    intTree.insert(20);
    intTree.insert(40);
    intTree.insert(60);
    intTree.insert(80);
    
    // Display the tree (inorder traversal)
    std::cout << "Inorder traversal: ";
    intTree.inorder_traversal();
    
    // Search for values
    std::cout << "Search for 40: " << (intTree.search(40) ? "Found" : "Not found") << std::endl;
    std::cout << "Search for 45: " << (intTree.search(45) ? "Found" : "Not found") << std::endl;
    
    // Create a binary tree of strings
    BinaryTree<std::string> stringTree;
    
    // Insert some strings
    stringTree.insert("grape");
    stringTree.insert("apple");
    stringTree.insert("orange");
    stringTree.insert("banana");
    stringTree.insert("watermelon");
    
    // Display the tree (inorder traversal)
    std::cout << "Inorder traversal of string tree: ";
    stringTree.inorder_traversal();
    
    return 0;
}