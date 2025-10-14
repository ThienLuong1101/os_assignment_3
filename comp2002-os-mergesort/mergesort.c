/**
 * This file implements parallel mergesort.
 */
#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include <pthread.h>
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */
void merge(int leftstart, int leftend, int rightstart, int rightend){
    int i = leftstart;      // index for left subarray
    int j = rightstart;     // index for right subarray
    int k = leftstart;      // index for merged array B
    
    // Merge the two subarrays into B
    while (i <= leftend && j <= rightend) {
        if (A[i] <= A[j]) {
            B[k] = A[i];
            i++;
        } else {
            B[k] = A[j];
            j++;
        }
        k++;
    }
    
    // Copy remaining elements from left subarray if any
    while (i <= leftend) {
        B[k] = A[i];
        i++;
        k++;
    }
    
    // Copy remaining elements from right subarray if any
    while (j <= rightend) {
        B[k] = A[j];
        j++;
        k++;
    }
    
    // Copy merged elements back from B to A
    memcpy(&A[leftstart], &B[leftstart], (rightend - leftstart + 1) * sizeof(int));
}

/* this function will be called by parallel_mergesort() as its base case. */
void my_mergesort(int left, int right){
    if (left < right) {
        int mid = left + (right - left) / 2;
        
        // Recursively sort left half
        my_mergesort(left, mid);
        
        // Recursively sort right half
        my_mergesort(mid + 1, right);
        
        // Merge the two sorted halves
        merge(left, mid, mid + 1, right);
    }
}

/* this function will be called by the testing program. */
void * parallel_mergesort(void *arg){
    struct argument *args = (struct argument *)arg;
    int left = args->left;
    int right = args->right;
    int level = args->level;
    
    // Base case: if we've reached the cutoff level, use serial merge sort
    if (level >= cutoff) {
        my_mergesort(left, right);
        // Only free if this is NOT the initial call from main
        if (level > 0) {
            free(args);
        }
        return NULL;
    }
    
    // Calculate midpoint
    int mid = left + (right - left) / 2;
    
    // Create arguments for left and right threads
    struct argument *left_args = buildArgs(left, mid, level + 1);
    struct argument *right_args = buildArgs(mid + 1, right, level + 1);
    
    // Create threads for left and right halves
    pthread_t left_thread, right_thread;
    
    pthread_create(&left_thread, NULL, parallel_mergesort, left_args);
    pthread_create(&right_thread, NULL, parallel_mergesort, right_args);
    
    // Wait for both threads to complete
    pthread_join(left_thread, NULL);
    pthread_join(right_thread, NULL);
    
    // Merge the two sorted halves
    merge(left, mid, mid + 1, right);
    
    // Only free if this is NOT the initial call from main
    if (level > 0) {
        free(args);
    }
    
    return NULL;
}

/* we build the argument for the parallel_mergesort function. */
struct argument * buildArgs(int left, int right, int level){
    struct argument *args = (struct argument *)malloc(sizeof(struct argument));
    if (args == NULL) {
        fprintf(stderr, "Error: malloc failed in buildArgs\n");
        exit(EXIT_FAILURE);
    }
    
    args->left = left;
    args->right = right;
    args->level = level;
    
    return args;
}