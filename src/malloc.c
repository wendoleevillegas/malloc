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

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes     */
   struct _block *next;  /* Pointer to the next _block of allocated memory      */
   struct _block *prev;  /* Pointer to the previous _block of allocated memory  */
   bool   free;          /* Is this _block free?                                */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3jMDEED                       */
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *last_allocated = NULL; // for Next Fit

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   //
   // While we haven't run off the end of the linked list and
   // while the current node we point to isn't free or isn't big enough
   // then continue to iterate over the list.  This loop ends either
   // with curr pointing to NULL, meaning we've run to the end of the list
   // without finding a node or it ends pointing to a free node that has enough
   // space for the request.
   // 
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

// BEST FIT - WORKS
// \TODO Put your Best Fit code in this #ifdef block
#if defined BEST && BEST == 0
   /** \TODO Implement best fit here */
   struct _block *best_fit = NULL;
   size_t best_size = SIZE_MAX;
   while (curr)
   {
      if (curr->free && curr->size >= size && curr->size < best_size)
      {
         best_fit = curr;
         best_size = curr->size;
      }
   
      *last = curr;
      curr = curr->next;
   }

   curr = best_fit;

#endif

// WORST FIT - WORKS
// \TODO Put your Worst Fit code in this #ifdef block
#if defined WORST && WORST == 0
   /** \TODO Implement worst fit here */
   struct _block *worst_fit = NULL;
   size_t worst_size = 0;
   
   while (curr)
   {
      if (curr->free && curr->size >= size && curr->size > worst_size)
      {
         worst_fit = curr;
         worst_size = curr->size;
      }
      
      *last = curr;
      curr = curr->next;
   }

   curr = worst_fit;

#endif

// NEXT FIT - WORKS
// \TODO Put your Next Fit code in this #ifdef block
#if defined NEXT && NEXT == 0
   /** \TODO Implement next fit here */
   if (last_allocated)
   {
      curr = last_allocated->next;
   }

   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr = curr->next;
      last_allocated = curr;
   }
   
   if (curr)
   {
      last_allocated = curr;
   }

#endif

   return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   
   num_blocks++;
   max_heap = max_heap + size;

   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */

   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: If the block found by findFreeBlock is larger than we need then:
            If the leftover space in the new block is greater than the sizeof(_block)+4 then
            split the block.
            If the leftover space in the new block is less than the sizeof(_block)+4 then
            don't split the block.
   */

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      num_grows++;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }

   // added
   if (next->size > size + sizeof(struct _block))
   {
      num_splits++;
      num_blocks++;
      struct _block *split = (struct _block *)((char *)next + size + sizeof(struct _block));
      split->size = next->size - size - sizeof(struct _block);
      split->next = next->next;
      split->prev = next;
      split->free = true;
      next->size = size;
      next->next = split;
      //num_splits++;
   }
   
   // added reuses
   else if(next != NULL && next->size >= size)
   {
      num_reuses++;
   }

   /* Mark _block as in use */
   next->free = false;

   num_mallocs++;
   num_requested += size;
   //num_blocks++;

   /* Return data address associated with _block to the user */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   //assert(curr->free == 0);
   curr->free = true;

   // Coalese blocks. if next block || prev block are free,
   // combine them with the block being freed

   struct _block *current = heapList;
   while(current != NULL)
   {
      if(current->free && current->next != NULL && current->next->free)
      {
         current->size += sizeof(struct _block) + current->next->size;
         current->next = current->next->next;

         num_coalesces++;
         num_blocks--;
      }

      else
      {
         current = current->next;
      }
   }

   // AI did not help in this section, had to redo this whole part
   /*
   if (curr->next && curr->next->free)
   {
      curr->size += sizeof(struct _block) + curr->next->size;
      curr->next = curr->next->next;


      if (curr->next)
      {
         curr->next->prev = curr;
      }
      
      num_coalesces++; // inc coalescing amount
   }

   // coalesce with prev block if it is free
   if (curr->prev && curr->prev->free)
   {
      curr->prev->size += sizeof(struct _block) + curr->size;
      curr->prev->next = curr->next;

      if (curr->next)
      {
         curr->next->prev = curr->prev;
      }
      
      num_coalesces++; // inc coalescing amount
      num_blocks--; // decrement block after coalescing
   }
   */

   num_frees++;
}

void *calloc( size_t nmemb, size_t size )
{
   size_t total_size = nmemb * size;
   
   void *ptr = malloc(total_size);
   if (ptr)
   {
      memset(ptr, 0, total_size);
   }
   
   return ptr;
}

void *realloc( void *ptr, size_t size )
{
   if (ptr == NULL)
   {
      return malloc(size);
   }
   if (size == 0)
   {
      free(ptr);
      return NULL;
   }

   struct _block *curr = BLOCK_HEADER(ptr);
   size_t old_size = curr->size;

   if (size <= old_size)
   {
      if (old_size - size >= sizeof(struct _block) + 4)
      {
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
   if (new_ptr)
   {
      memcpy(new_ptr, ptr, old_size);
      free(ptr);
   }

   return new_ptr;
}



/* vim: IENTRTMzMjAgU3ByaW5nIDIwM001= ----------------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
