#include "../include/sort_interface.h"
#include <stdint.h>
//needed for the quicksort since i want to avoid recursion as the code is embedded-ready.
#define MAX_ELEMENTS 1024
#define STACK_SIZE 64
//temporary pointer swap function
static inline void swap_ptr(int32_t* a, int32_t* b) {
    int32_t temp = *a;
    *a = *b;
    *b = temp;
}
// Basic bubble sort implementation with visual callbacks
__declspec(dllexport) void bubble_sort_basic(int32_t* array, int32_t size, VisualCallback onSwap, VisualCallback onCompare) {
    for (uint32_t    i = 0; (uint32_t) (i < size - 1); i++) {
        for (uint32_t j = 0; j < size - i - 1; j++) {
            // Access current and next elements
            int32_t curr = *(array + j);
            int32_t next = *(array + j + 1);
            //send signal compare to visualizer
            if (onCompare) {
                onCompare(j, j + 1, array);
            }
            //basic bubble sort logic
            if (curr > next) {
                
                swap_ptr((array + j), (array + j + 1));

              //send signal swap to visualizer
                if (onSwap) {
                    onSwap(j, j + 1, array);
                }
            }
        }
    }
}

// partition function for quicksort
static int32_t partition(int32_t* base_arr, int32_t low, int32_t high, VisualCallback onSwap, VisualCallback onCompare) {
    int32_t pivot = *(base_arr + high); 
    int32_t i = low - 1;

    for (int32_t j = low; j < high; j++) {
        if (onCompare) onCompare(j, high, base_arr);

        if (*(base_arr + j) < pivot) {
            i++;
            swap_ptr((base_arr + i), (base_arr + j));
            
            if (onSwap) onSwap(i, j, base_arr);
        }
    }
    
    swap_ptr((base_arr + i + 1), (base_arr + high));
    if (onSwap) onSwap(i + 1, high, base_arr);
    
    return i + 1;
}

__declspec(dllexport) void quick_sort_iterative(int32_t* arr, int32_t n, VisualCallback onSwap, VisualCallback onCompare) {
    if (arr == NULL || n <= 1 || n > MAX_ELEMENTS) return;
    // Using an explicit stack to avoid recursion
    static int32_t static_stack[STACK_SIZE];
    int32_t top = -1;
    
    static_stack[++top] = 0;
    static_stack[++top] = n - 1;
    
    while (top >= 0) {
        int32_t high = static_stack[top--];
        int32_t low = static_stack[top--];

        int32_t p = partition(arr, low, high, onSwap, onCompare);

        if (top + 4 >= STACK_SIZE) {
            break; 
        }

        if (p - 1 > low) {
            static_stack[++top] = low;
            static_stack[++top] = p - 1;
        }

        if (p + 1 < high) {
            static_stack[++top] = p + 1;
            static_stack[++top] = high;
        }
    }
}