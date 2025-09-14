#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

void printStatistics(void) {
    printf("\nheap management statistics\n");
    printf("mallocs:\t%d\n", num_mallocs);
    printf("frees:\t\t%d\n", num_frees);
    printf("reuses:\t\t%d\n", num_reuses);
    printf("grows:\t\t%d\n", num_grows);
    printf("splits:\t\t%d\n", num_splits);
    printf("coalesces:\t%d\n", num_coalesces);
    printf("blocks:\t\t%d\n", num_blocks);
    printf("requested:\t%d\n", num_requested);
    printf("max heap:\t%d\n", max_heap);
}

struct _block {
    size_t size;
    struct _block *next;
    struct _block *prev;
    bool free;
    char padding[3];
};

struct _block *heapList = NULL;
struct _block *last_allocated = NULL;  // For Next Fit

struct _block *findFreeBlock(struct _block **last, size_t size) {
    struct _block *curr = heapList;

#if defined FIT && FIT == 0
    // First Fit
    while (curr && !(curr->free && curr->size >= size)) {
        *last = curr;
        curr  = curr->next;
    }
#endif

#if defined BEST && BEST == 0
    // Best Fit
    struct _block *best_fit = NULL;
    size_t best_size = SIZE_MAX;
    while (curr) {
        if (curr->free && curr->size >= size && curr->size < best_size) {
            best_fit = curr;
            best_size = curr->size;
        }
        *last = curr;
        curr = curr->next;
    }
    curr = best_fit;
#endif

#if defined WORST && WORST == 0
    // Worst Fit
    struct _block *worst_fit = NULL;
    size_t worst_size = 0;
    while (curr) {
        if (curr->free && curr->size >= size && curr->size > worst_size) {
            worst_fit = curr;
            worst_size = curr->size;
        }
        *last = curr;
        curr = curr->next;
    }
    curr = worst_fit;
#endif

#if defined NEXT && NEXT == 0
    // Next Fit
    if (last_allocated) {
        curr = last_allocated->next;
    }
    while (curr && !(curr->free && curr->size >= size)) {
        *last = curr;
        curr = curr->next;
        if (!curr) {
            curr = heapList;
        }
    }
    if (curr) {
        last_allocated = curr;
    }
#endif

    return curr;
}

struct _block *growHeap(struct _block *last, size_t size) {
    struct _block *curr = (struct _block *)sbrk(0);
    struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

    assert(curr == prev);

    if (curr == (struct _block *)-1) {
        return NULL;
    }

    if (heapList == NULL) {
        heapList = curr;
    }

    if (last) {
        last->next = curr;
    }

    curr->size = size;
    curr->next = NULL;
    curr->free = false;
    return curr;
}

void *malloc(size_t size) {
    if (atexit_registered == 0) {
        atexit_registered = 1;
        atexit(printStatistics);
    }

    size = ALIGN4(size);

    if (size == 0) {
        return NULL;
    }

    struct _block *last = heapList;
    struct _block *next = findFreeBlock(&last, size);

    if (next == NULL) {
        next = growHeap(last, size);
    }

    if (next == NULL) {
        return NULL;
    }

    if (next->size > size + sizeof(struct _block)) {
        struct _block *split = (struct _block *)((char *)next + size + sizeof(struct _block));
        split->size = next->size - size - sizeof(struct _block);
        split->next = next->next;
        split->prev = next;
        split->free = true;
        next->size = size;
        next->next = split;
        num_splits++;
    }

    next->free = false;
    num_mallocs++;
    num_requested += size;
    num_blocks++;

    return BLOCK_DATA(next);
}

void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    struct _block *curr = BLOCK_HEADER(ptr);
    assert(curr->free == 0);
    curr->free = true;
    num_frees++;

    if (curr->next && curr->next->free) {
        curr->size += sizeof(struct _block) + curr->next->size;
        curr->next = curr->next->next;
        if (curr->next) {
            curr->next->prev = curr;
        }
        num_coalesces++;
    }

    if (curr->prev && curr->prev->free) {
        curr->prev->size += sizeof(struct _block) + curr->size;
        curr->prev->next = curr->next;
        if (curr->next) {
            curr->next->prev = curr->prev;
        }
        num_coalesces++;
    }

    num_blocks--;
}

void *calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    struct _block *curr = BLOCK_HEADER(ptr);
    size_t old_size = curr->size;

    if (size <= old_size) {
        if (old_size - size >= sizeof(struct _block) + 4) {
            struct _block *split = (struct _block *)((char *)curr + size + sizeof(struct _block));
            split->size = old_size - size - sizeof(struct _block);
            split->next = curr->next;
            split->prev = curr;
            split->free = true;
            curr->size = size;
            curr->next = split;
            num_splits++;
        }
        return ptr;
    }

    void *new_ptr = malloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size);
        free(ptr);
    }

    return new_ptr;
}
