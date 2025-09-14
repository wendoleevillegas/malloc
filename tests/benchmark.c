#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "malloc.h" // Assuming your custom allocator implementation is in malloc.h and malloc.c

#define NUM_BLOCKS 1000
#define MAX_SIZE 1024
#define MIN_SIZE 16

void *blocks[NUM_BLOCKS];

void basic_stress_test();
void random_allocation_test();
void sequential_growth_test();
void fragmentation_test();
void reallocation_stress_test();

// Functions to be implemented in malloc.c for tracking memory stats
//size_t get_total_free_memory();
//size_t get_largest_free_block();

int main()
{
    printf("\n--- Basic Stress Test ---\n");
    basic_stress_test();

    printf("\n--- Random Allocation Test ---\n");
    random_allocation_test();

    printf("\n--- Sequential Growth Test ---\n");
    sequential_growth_test();

    printf("\n--- Fragmentation Test ---\n");
    fragmentation_test();

    printf("\n--- Reallocation Stress Test ---\n");
    reallocation_stress_test();

    return 0;
}

void basic_stress_test() //performance test   ---  tests/benchmark - system malloc
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    //void *blocks[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        blocks[i] = malloc(32);
    }
    for (int i = 0; i < NUM_BLOCKS; i += 2)
    {
        free(blocks[i]);
    }
    for (int i = 0; i < NUM_BLOCKS / 2; i++)
    {
        blocks[i] = malloc(32);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1e6);
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);
}

void random_allocation_test()
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        size_t size = (rand() % (MAX_SIZE - MIN_SIZE + 1)) + MIN_SIZE;
        blocks[i] = malloc(size);
    }
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rand() % 2 == 0)
        {
            free(blocks[i]);
        }
    }
    for (int i = 0; i < NUM_BLOCKS / 2; i++)
    {
    blocks[i] = malloc((rand() % (MAX_SIZE - MIN_SIZE + 1)) + MIN_SIZE);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1e6);
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);
}

void sequential_growth_test()
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t size = MIN_SIZE;
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        blocks[i] = malloc(size);
        size *= 2;
        if (size > MAX_SIZE)
        {
            size = MIN_SIZE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1e6);
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);
}

void fragmentation_test()
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    void *large_block = malloc(512 * NUM_BLOCKS);
    void *small_blocks[NUM_BLOCKS];

    // Allocate small blocks
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
    small_blocks[i] = malloc(32);
    }

    // Free every alternate block to create fragmentation
    for (int i = 0; i < NUM_BLOCKS; i += 2)
    {
    free(small_blocks[i]);
    }

    // Attempt to allocate another large block
    void *new_large_block = malloc(512 * NUM_BLOCKS);

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time
    double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1e6);
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);

    // Calculate fragmentation percentage using updated allocator info
    //size_t total_free_memory = get_total_free_memory(); // Function to get total free memory
    //size_t largest_free_block = get_largest_free_block(); // Function to get largest free block

    /*if (total_free_memory > 0)
    {
        double fragmentation_percentage = (1.0 - ((double)largest_free_block / total_free_memory)) * 100.0;
        printf("Fragmentation Percentage: %.2f%%\n", fragmentation_percentage);
    }
    else
    {
        printf("No free memory available.\n");
    }*/

    if (new_large_block == NULL)
    {
        printf("Fragmentation prevented allocation of new large block.\n");
    }

    // Clean up
    free(large_block);
    if (new_large_block)
    {   
        free(new_large_block);
    }
}

void reallocation_stress_test()
{
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        blocks[i] = malloc(64);
    }
    for (int i = 0; i < NUM_BLOCKS; i += 2)
    {
        blocks[i] = realloc(blocks[i], 128);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed_time = ((end.tv_sec - start.tv_sec) * 1000.0) + ((end.tv_nsec - start.tv_nsec) / 1e6);
    printf("Elapsed time: %.2f milliseconds\n", elapsed_time);

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        free(blocks[i]);
    }
}
