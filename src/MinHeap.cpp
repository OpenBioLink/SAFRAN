#include "MinHeap.h"

// Constructor: Builds a heap from a given array a[] of given size 
MinHeap::MinHeap(int cap)
{
    heap_size = cap;
    capacity = cap;
    harr = new std::pair<int, float50>[cap];
    for (int i = 0; i < cap; i++) {
        harr[i] = std::make_pair(-1, 0.0);
    }
}

MinHeap::~MinHeap()
{
    delete[] harr;
}

// Inserts a new key 'k' 
void MinHeap::insertKey(std::pair<int, float50> k)
{
    if (heap_size == capacity)
    {
        std::cout << "\nOverflow: Could not insertKey\n";
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

std::pair<int, float50> MinHeap::extractMin()
{
    if (heap_size <= 0)
        return std::make_pair(-1, 0.0);
    if (heap_size == 1)
    {
        heap_size--;
        return harr[0];
    }

    // Store the minimum value, and remove it from heap 
    std::pair<int, float50> root = harr[0];
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
void  MinHeap::swapperoni(std::pair<int, float50>* x, std::pair<int, float50>* y)
{
    std::pair<int, float50> temp = *x;
    *x = *y;
    *y = temp;
}