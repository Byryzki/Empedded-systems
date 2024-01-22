#include <stdio.h>

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void bubbleSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] < arr[j + 1]) {
                swap(&arr[j], &arr[j + 1]);
            }
        }
    }
}

int main() {
    int arr[4] = {6, 2, 8, 15};

    // Sort the array
    bubbleSort(arr, 4);

    // Display the sorted array
    printf("Sorted array: ");
    for (int i = 0; i < 4; i++) {
        printf("%d ", arr[i]);
    }

    return 0;
}