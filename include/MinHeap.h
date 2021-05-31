#ifndef MIN_H
#define MIN_H

// A C++ program to demonstrate common Binary Heap Operations 
#include<iostream> 
#include<climits> 
#include "boost/multiprecision/cpp_bin_float.hpp"

typedef boost::multiprecision::cpp_bin_float_50 float50;

// Prototype of a utility function to swap two integers 
void swap(int* x, int* y);

// A class for Min Heap 
class MinHeap
{
    std::pair<int, float50>* harr; // pointer to array of elements in heap 
    int capacity; // maximum possible size of min heap 
    int heap_size; // Current number of elements in min heap 
public:
    // Constructor 
    MinHeap(int capacity);
    ~MinHeap();

    int parent(int i) { return (i - 1) / 2; }

    // to get index of left child of node at index i 
    int left(int i) { return (2 * i + 1); }

    // to get index of right child of node at index i 
    int right(int i) { return (2 * i + 2); }

    // Returns the minimum key (key at root) from min heap 
    std::pair<int, float50> getMin() { return harr[0]; }

    std::pair<int, float50> extractMin();

    std::pair<int, float50> get(int index) { return harr[index]; }

    // Inserts a new key 'k' 
    void insertKey(std::pair<int, float50> k);
    void swapperoni(std::pair<int, float50>* x, std::pair<int, float50>* y);
    void deleteMin();
    void MinHeapify(int i);
    int getSize();
};

#endif