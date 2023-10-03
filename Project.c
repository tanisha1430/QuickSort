#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int partition(int arr[], int low, int high) {
    int pivot = arr[low];
    int i = low;

    for (int j = low + 1; j <= high; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[low], &arr[i]);
    return i;
}

void quicksort(int arr[], int low, int high) {
    if (low < high) {
        int pivot_index = partition(arr, low, high);

        quicksort(arr, low, pivot_index - 1);
        quicksort(arr, pivot_index + 1, high);
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        int n;
        printf("Enter the number of elements: ");
        scanf("%d", &n);

        int* data = (int*)malloc(n * sizeof(int));

        printf("Enter the elements: ");
        for (int i = 0; i < n; i++) {
            scanf("%d", &data[i]);
        }

        // Broadcast the number of elements to all processes
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // Scatter the data among processes
        int local_size = n / size;
        int* local_data = (int*)malloc(local_size * sizeof(int));
        MPI_Scatter(data, local_size, MPI_INT, local_data, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        // Perform local Quick Sort
        quicksort(local_data, 0, local_size - 1);

        // Gather sorted subarrays from all processes
        MPI_Gather(local_data, local_size, MPI_INT, data, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        // Merge the sorted subarrays (not shown in this simplified example)
        // You can use a sequential or parallel merging algorithm here

        if (rank == 0) {
            printf("Sorted array: ");
            for (int i = 0; i < n; i++) {
                printf("%d ", data[i]);
            }
            printf("\n");
        }

        free(data);
        free(local_data);
    } else {
        int n;
        MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

        int local_size = n / size;
        int* local_data = (int*)malloc(local_size * sizeof(int));
        MPI_Scatter(NULL, local_size, MPI_INT, local_data, local_size, MPI_INT, 0, MPI_COMM_WORLD);

        quicksort(local_data, 0, local_size - 1);

        MPI_Gather(local_data, local_size, MPI_INT, NULL, 0, MPI_INT, 0, MPI_COMM_WORLD);

        free(local_data);
    }

    MPI_Finalize();
	return 0;
}
