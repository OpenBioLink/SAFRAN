#ifndef MIN_H
#define MIN_H

// A C++ program to demonstrate common Binary Heap Operations 
#include<iostream> 
#include<climits> 
using namespace std;

// Prototype of a utility function to swap two integers 
void swap(int* x, int* y);

// A class for Min Heap 
class MinHeap
{
    std::pair<int,double>* harr; // pointer to array of elements in heap 
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

// Constructor: Builds a heap from a given array a[] of given size 
MinHeap::MinHeap(int cap)
{
    heap_size = cap;
    capacity = cap;
    harr = new std::pair<int, double>[cap];
    for (int i = 0; i < cap; i++) {
        harr[i] = std::make_pair(-1, 0.0);
    }
}

MinHeap::~MinHeap()
{
    delete[] harr;
}

// Inserts a new key 'k' 
void MinHeap::insertKey(std::pair<int, double> k)
{
    if (heap_size == capacity)
    {
        cout << "\nOverflow: Could not insertKey\n";
        return;
    }

    // First insert the new key at the end 
    heap_size++;
    int i = heap_size - 1;
    harr[i] = k;

    // Fix the min heap property if it is violated 
    while (i != 0 && harr[parent(i)].second > harr[i].second)
    {
        swapperoni(&harr[i], &harr[parent(i)]);
        i = parent(i);
    }
}

int MinHeap::getSize() {
    return heap_size;
}

void MinHeap::deleteMin()
{
    if (heap_size == 1)
    {
        heap_size--;
        return;
    }

    harr[0] = harr[heap_size - 1];
    heap_size--;
    MinHeapify(0);
}

std::pair<int, double> MinHeap::extractMin()
{
    if (heap_size <= 0)
        return std::make_pair(-1, 0.0);
    if (heap_size == 1)
    {
        heap_size--;
        return harr[0];
    }

    // Store the minimum value, and remove it from heap 
    std::pair<int, double> root = harr[0];
    harr[0] = harr[heap_size - 1];
    heap_size--;
    MinHeapify(0);

    return root;
}

void MinHeap::MinHeapify(int i)
{
    int l = left(i);
    int r = right(i);
    int smallest = i;
    if (l < heap_size && harr[l].second < harr[i].second)
        smallest = l;
    if (r < heap_size && harr[r].second < harr[smallest].second)
        smallest = r;
    if (smallest != i)
    {
        swapperoni(&harr[i], &harr[smallest]);
        MinHeapify(smallest);
    }
}

// A utility function to swap two elements 
void  MinHeap::swapperoni(std::pair<int, double>* x, std::pair<int, double>* y)
{
    std::pair<int, double> temp = *x;
    *x = *y;
    *y = temp;
}

#endif