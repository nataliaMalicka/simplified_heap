#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "myheap.h"

#define HEADER_SIZE 8

/*
 * Struct used to represent the heap.
 */
struct myheap {
    long size;                /* Size of the heap in bytes. */
    void *start;             /* Start address of the heap area. */
};

/*
 * Determine whether or not a block is in use.
 */
static int block_is_in_use(void *block_start) {

  return 1 & *((long *) block_start);
}

/*
 * Return the size of a block.
 */
static int get_block_size(void *block_start) {

  long *header = block_start;
  // remove the last bit from header (i.e, the alloc bit) and return the result (i.e., the block size)
  return *header & 0xfffffffffffffffe;
}

/*
 * Return the size of the payload of a block.
 */
static int get_payload_size(void *block_start) {
  
  return get_block_size(block_start) - HEADER_SIZE * 2;
}

/*
 * Find the start of the block, given a pointer to the payload.
 */
static void *get_block_start(void *payload) {
  
  return payload - HEADER_SIZE;
}

/*
 * Find the payload, given a pointer to the start of the block.
 */
static void *get_payload(void *block_start) {
  
  return block_start + HEADER_SIZE;
}

/*
 * Set the size of a block, and whether or not it is in use. Remember
 * each block has two copies of the header (one at each end).
 */
static void set_block_header(void *block_start, int block_size, int in_use) {
  
  long header_value = block_size | in_use;
  long *header_position = block_start;
  long *trailer_position = block_start + block_size - HEADER_SIZE;
  *header_position  = header_value;
  *trailer_position = header_value;

  //printf("block_start: %p \n", block_start);
  //printf("block_size: %d \n", block_size);
  //if (in_use == 1) {
    //printf("in use\n");
  //} else {
    //printf("free\n");
  //}
}


/*
 * Find the start of the next block.
 */
static void *get_next_block(void *block_start) {
  
  return block_start + get_block_size(block_start);
}

/*
 * Find the start of the previous block.
 */
static void *get_previous_block(void *block_start) {
  
  return block_start - get_block_size(block_start - HEADER_SIZE);
}

/*
 * Determine whether or not the given block is at the front of the heap.
 */
static int is_first_block(struct myheap *h, void *block_start) {
  
  return block_start == h->start;
}

/*
 * Determine whether or not the given block is at the end of the heap.
 */
static int is_last_block(struct myheap *h, void *block_start) {
  
  return get_next_block(block_start) == h->start + h->size;
}

/*
 * Determine whether or not the given address is inside the heap
 * region. Can be used to loop through all blocks:
 *
 * for (blk = h->start; is_within_heap_range(h, blk); blk = get_next_block(blk)) ...
 */
static int is_within_heap_range(struct myheap *h, void *addr) {
  
  return addr >= h->start && addr < h->start + h->size;
}

/*
 * Coalesce free space for single block pair by
 * joining first_block_start and its consecutive next block
 * if and only if both blocks are free and first_block_start
 * has a next block in the heap. 
 *
 * NOTE: This function can return NULL, but if your design would benefit from
 * it returning a pointer to some block, then you are free to have it do so.
 */
static void *coalesce(struct myheap *h, void *first_block_start) {
  


  /* TO BE COMPLETED BY THE STUDENT. */
  if (!is_last_block(h, first_block_start) 
  && !block_is_in_use(first_block_start) 
  && !block_is_in_use(get_next_block(first_block_start))) {
  
    //printf("CoSt Start + %p \n", first_block_start);
    //printf("CoSt Size + %d\n", get_block_size(first_block_start));

    int size = get_block_size(first_block_start);
    int nextAddress = get_next_block(first_block_start);

    int totalChunkSize = get_block_size(first_block_start) + get_block_size(get_next_block(first_block_start));
    
    //printf("CoHappening - %d\n", totalChunkSize);
    
    set_block_header(first_block_start, totalChunkSize, 0);

    //printf("CoEnd Start + %p\n", first_block_start);
    //printf("CoEnd Size + %d\n", get_block_size(first_block_start));
  }



  return NULL;
}

/*
 * Determine the size of the block we need to allocate given the size
 * the user requested. Don't forget we need space for the header and
 * footer, and that the block's actual payload size must be a multiple
 * of HEADER_SIZE.
 */
static int get_size_to_allocate(int user_size) {
  /* TO BE COMPLETED BY THE STUDENT. */
  int blockSize = 0;

  if (user_size % HEADER_SIZE == 0) {
    return user_size + (HEADER_SIZE * 2);
  } else {
    int remainder = user_size - (user_size % HEADER_SIZE);
    return remainder + HEADER_SIZE + (HEADER_SIZE * 2);
  }
}

/*
 * Checks if the block can be split. It can split if the left over
 * bytes after the split (current block size minus needed size) are
 * enough for a new block, i.e., there is enough space left over for a
 * header, trailer and some payload (i.e., at least 3 times
 * HEADER_SIZE). If it can be split, splits the block in two and marks
 * the first as in use, the second as free. Otherwise just marks the
 * block as in use. Returns the payload of the block marked as in use.
 */
static void *split_and_mark_used(struct myheap *h, void *block_start, int needed_size) {
  //printf("Split Size: %d\n", needed_size);

  /* TO BE COMPLETED BY THE STUDENT. */
  int minSize = HEADER_SIZE * 3;
  int currBlockSize = get_block_size(block_start);
  int reqSize = get_size_to_allocate(needed_size);
  int remainder = currBlockSize - reqSize;
  int inUse = block_is_in_use(block_start);

  if (inUse == 0) {
    if (currBlockSize - reqSize >= minSize) {
      //split in two:
      set_block_header(block_start, reqSize, 1); //1st block, in use
      set_block_header((block_start + reqSize), (currBlockSize - reqSize), 0); //2nd block, free
    } else {
      set_block_header(block_start, currBlockSize, 1); //set block as in use
    }
  }
  return get_payload(block_start);
}

/*
 * Create a heap that is "size" bytes large.
 */
struct myheap *heap_create(unsigned int size)
{
  /* Allocate space in the process' actual heap */
  void *heap_start = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (heap_start == (void *) -1) return NULL;
  
  /* Use the first part of the allocated space for the heap header */
  struct myheap *h = heap_start;
  h->size = size - sizeof(struct myheap);
  h->start = heap_start + sizeof(struct myheap);

  /* Initializes one big block with the full space in the heap. Will
     be split in the first malloc. */
  set_block_header(h->start, h->size, 0);
  return h;
}

/*
 * Free a block on the heap h. Also attempts to join (coalesce) the
 * block with the previous and the next block, if they are also free.
 */
void myheap_free(struct myheap *h, void *payload) {
  
  /* TO BE COMPLETED BY THE STUDENT. */
  void* blockStart = get_block_start(payload);
  

  int sizeBlock = get_block_size(blockStart);
  //printf("Free Addr %p", blockStart);
  //printf("Free Addr %d", sizeBlock);
  set_block_header(blockStart, sizeBlock, 0);
  
  //coalesce(h, blockStart);
  //coalesce(h, get_previous_block(blockStart));
}

/*
 * Malloc a block on the heap h. 
 * Return a pointer to the block's payload in case of success, 
 * or NULL if no block large enough to satisfy the request exists.
 */
void *myheap_malloc(struct myheap *h, unsigned int user_size) {
  
  /* TO BE COMPLETED BY THE STUDENT. */
  void* curr = h->start;

  while (curr != NULL) {
    if (!block_is_in_use(curr) && (get_block_size(curr) >= get_size_to_allocate(user_size))) {
      //printf("heap malloc\n");
      return split_and_mark_used(h, curr, user_size);//ret payload ptr
    } else {
      if (!is_last_block(h, curr)) {
        curr = get_next_block(curr);
      } else {
        return NULL;
      }
      //printf("get next block to malloc\n");
    }
  }
  return NULL;
}
