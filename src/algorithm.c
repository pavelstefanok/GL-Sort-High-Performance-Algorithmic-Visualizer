#include "../include/sort_interface.h"
//temporary pointer swap function
static void swap_ptr(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
// Basic bubble sort implementation with visual callbacks
__declspec(dllexport) void bubble_sort_basic(int* array, int size, VisualCallback onSwap, VisualCallback onCompare) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {

            //send signal compare to visualizer
            if (onCompare) {
                onCompare(j, j + 1, array);
            }
            //basic bubble sort logic
            if (*(array + j) > *(array + j + 1)) {
                
                swap_ptr((array + j), (array + j + 1));

              //send signal swap to visualizer
                if (onSwap) {
                    onSwap(j, j + 1, array);
                }
            }
        }
    }
}