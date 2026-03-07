#ifndef SORT_INTERFACE_H
#define SORT_INTERFACE_H

// Define a callback type for visualization
typedef void (*VisualCallback)(int i, int j, int* currentArray);

#ifdef __cplusplus
extern "C" {
#endif

// Declare the sorting function with visual callbacks
__declspec(dllexport) void bubble_sort_basic(int* arr, int n, VisualCallback onSwap, VisualCallback onCompare);

#ifdef __cplusplus
}
#endif

#endif