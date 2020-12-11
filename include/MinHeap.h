#ifndef MIN_H
#define MIN_H

// A C++ program to demonstrate common Binary Heap Operations 
#include<iostream> 
#include<climits> 

// Prototype of a utility function to swap two integers 
void swap(int* x, int* y);

// A class for Min Heap 
class MinHeap
{
    std::pair<int, double>* harr; // pointer to array of elements in heap 
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
    std::pair<int, double> getMin() { return harr[0]; }

    std::pair<int, double> extractMin();

    std::pair<int, double> get(int index) { return harr[index]; }

    // Inserts a new key 'k' 
    void insertKey(std::pair<int, double> k);
    void swapperoni(std::pair<int, double>* x, std::pair<int, double>* y);
    void deleteMin();
    void MinHeapify(int i);
    int getSize();
};

#endif