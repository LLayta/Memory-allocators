# Memory-allocators
Educational repository to briefly go over practical implementations of memory allocators and theoretical background of them.

# Contents 
[Why?](#Why?)
[Implementations](#Implementations)

# Why?
Why do we care about memory allocators? Memory allocators are an interface that allow us to preform *dynamic memory allocation* which is when we store long lived values in a programs' memory. The size of these allocations are determined at run-time. 

# How?
Memory allocators (typically) utilize a part of processes' memory space called the "heap". We can dynamically allocate data onto the heap in 2 main ways on linux:
  * mmap system call: Maps a chunk of memory that is utilized by our program that we store in between the data segment (.rodata, .text and what not) and the stack. See figure.1 for visualization.
  * sbrk function: Alters the program break to allocate space directly above the data segment. (Program break is a pointer to the top of the data segment on the first allocation but after allocations it'll be a pointer to the top of the heap). See figure.1 for visualization.

# Addressing space visualization:
![Addressing space](imgs/figure1.png)
figure1.

The "memory-mapped region" stores mmap'd chunks.
The section labeled "heap" stores the sbrk allocated chunks.

Sbrk is ideal for allocating smaller chunks because it consists of simply adding or subtracting from a pointer.
Mmap is ideal for really large chunk allocations that are around 4096 bytes worth of allocation space.

# What's the heap?
We threw the word "heap" around a lot, but what actually is the heap? The heap is a section of memory in our process that is constructed through the utilization of those functions / system calls we mentioned previously. Values stored on the heap are stored at run-time.

# Fragmentation
A concept we need to preface over before stepping into memory allocator implementations would be fragementation. This is a major trade-off when it comes to designing allocators. Fragmentation is a situation where we use memory inefficiently, we either have to go through extra measures to get the right sized chunks return or we end up allocating too much memory. 2 main types of fragmentation are such:
  * External fragmentation: Resulting chunk had to have been split to satisfy the size constraint. 
  * Internal fragmnetation: Resulting chunk was larger than what requested, leading to unused memory.

# Types of memory allocators:
So lets get into what they actually look in code and look at types of memory allocators.

## Linear allocator / bump allocators / arena allocators:
Linear allocators are allocators that allocate a chunk of memory deteremined by a given size that you index into with a pointer. You generally keep track of 3 different pointers. A pointer to the start of the allocated buffer, a current pointer that keeps track of the current pointer we have into the memory space. And an end pointer to make sure we don't write the current pointer past the memory space we defined.

A general structure for a linear allocation would be as such:

```c
typedef struct _linear_allocator {
  char *start, *curr, *end;
} Linear_allocator;
```

### Allocation:
The allocation would consist of altering the ``curr`` pointer by some amount

### Deallocation:
The deallocation would be resetting the pointers. We can't individually free allocations, we have to free the entire chunk at the same time. It would look like ``start = curr = end = 0;``

### Pros:
Extremely fast and a nice layer of abstraction which provides an easy use where instead of doing multiple allocations and storing information, we allocate 1 big chunk and index into it.

### Cons:
We can't individually free allocations in any order. We have to reset the entire chunk. 

## Free list allocators:
Free-list allocators are a technique for memory allocators where we store the allocation chunks in a linked list. The linked list nodes would generally contain a size field (keeps track of the size of the chunk), is freed field (to determine if the currently looked at chunk is free or not), and a next pointer (next node in the linked list). Now there are 2 types of free-list allocators, those being: explicit and implicit free lists.

* Implicit free list: Implicitly defines a structure of chunks that strictly points to free'd nodes.
* Explicit free list: Explicitly defines a structure of chunks that contain free and used nodes and are differentiated between with a flag determining if the currently loked at node is free or not.

### Allocation:
Allocation would consist of inserting these chunk like structures defined as: 
```c
typedef struct _chunk_t {
  size_t size;
  bool is_used;
  struct _chunk_t *fd;
} chunk_t;
```

Upon initial allocation you'd mmap or sbrk a new chunk and insert it at the head later allocations you'll search the linked list of chunks to find a "free node". The algorithms for finding these free chunks are called free chunk searching algorithms. Some of these algorithms are such:
  * First fit: Searches the free list for the first possible chunk it can take. This isn't ideal for fragmentation because we can find a really large node that satisfies the requirement size while there could possibly be smaller chunks to use.
  * Best fit: Searches the free list for the most space optimal chunk to take. Ideal for fragmentation but more complex to implement.

### Deallocation:
Deallocation would simply be fetching the chunk and setting the "is_used" field to false.

### Pros:
Pretty practical and opens up to a ton of optimizations leading to really fast and space efficient memory allocators.

### Cons:
A rather complex implementation that requires a linked list implementation, there are also a huge variety of different ways to implement them.

### Notes:
Tons of modern memory allocators such as ``ptmalloc`` and ``dlmalloc`` use this type of allocator. Some allocators implement bins to store certain lengths of chunk ranges for quick access in the free list. Some implement seperate linked lists for different sizes for fast access to free chunks. Some implement a binary search tree that orders based on chunk size or chunk address. A lot of allocators implement block splitting from recently fetched searching operations. A common optimization that ``free`` would implement would be block coalescing, where we combined adjacent free chunks into a big free chunk and mark it as new.

## Other types
Some types I haven't implemented but exist:
 * Buddy allocators
 * Stack allocators
 * Pool allocators (extremely practical)
 * Fixed size allocators
 * Slab allocators

# Why would we write our own?
Why would we write our own? Given memory allocator interfaces such as: ``malloc / free()`` and ``new / delete`` are designed to fit most general cases. This is very commonly done in video game engines and other areas of software engineering where hundreds of allocations are preformed.

# Implementations:

## Linear allocator:
```c
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define align_to_word(size) (size + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1)

#define LASSERT(condition, msg) \
    do \
    { \
        if(!(condition)) \
        { \
            fprintf(stderr, \
		"[*] Process terminated! 'LASSERT()' failed!\n" \
		"Condition\t: {%s}\n" \
	        "Function\t: {%s}\n" \
		"Failed in file\t: {%s}\n" \
	        "At line \t: {%d}\n", #condition, __func__, __FILE__, __LINE__); \
            fprintf(stderr, "Debug log: %s\n", msg); \
            exit(1); \
        } \
    } while(0) \


typedef struct _Allocator_t {
    size_t capacity;
    char *start, *end, *curr;
} Allocator_t;

Allocator_t *new_allocator(size_t capacity) {
    LASSERT(capacity, "Invalid allocation size!");

    Allocator_t *alloc = calloc(1, sizeof(*alloc));
    
    LASSERT(alloc, "Failed to allocate space for struct!");
    
    alloc->start = malloc(capacity);

    LASSERT(alloc->start, "Failed to allocate space for buffer!");

    alloc->capacity = capacity;
    alloc->curr = alloc->start;
    alloc->end = alloc->start + capacity;

    return alloc;
}

void *allocator_alloc(Allocator_t *alloc, size_t offset) {
    LASSERT(alloc && offset, "Invalid parameter(s)!");

    offset = align_to_word(offset);

    if(offset > alloc->capacity) {
        return NULL;
    }

    LASSERT(offset <= alloc->capacity, "Allocating more space than initialized with!");
    
    if(alloc->curr + offset <= alloc->end) {
        char *tmp = alloc->curr;
        alloc->curr += offset;
        
        return tmp;
    }

    return NULL;
}

void allocator_reset(Allocator_t *alloc) {
    LASSERT(alloc, "Trying to reset an invalid struct!");
    
    alloc->curr  = NULL;
    alloc->end   = NULL;
    alloc->start = NULL;
}

void delete_allocator(Allocator_t *alloc) {
    LASSERT(alloc, "Trying to free an unallocated struct!");

    free(alloc->start);
    free(alloc);
}
```

Example usage:
```c
int main(void) {
       Allocator_t *alloc = new_allocator(100);
    
    int *ptr1 = allocator_alloc(alloc, 5 * sizeof(int));
    for(int i = 0; i < 5; ++i) {
        ptr1[i] = i;
        printf("Ptr1[%d] => %d\n", i, ptr1[i]);
    }

    putchar('\n');

    char char_arr[] = { 'a', 'b', 'c', 'e', 'd' },
    *ptr2 = allocator_alloc(alloc, 5);
    
    for(int i = 0; i < 5; ++i) {
        ptr2[i] = char_arr[i];
        printf("Ptr2[%d] => %c\n", i, ptr2[i]);
    }

    putchar('\n');

    double double_arr[] = { 10.00, 20.00, 30.00, 40.00, 50.00 },
    *ptr3 = allocator_alloc(alloc, 5 * sizeof(double));

    for(int i = 0; i < 5; ++i) {
        ptr3[i] = double_arr[i];
        printf("Ptr3[%d] => %lf\n", i, ptr3[i]);
    }

    delete_allocator(alloc);
}
```

## Explicit free list
```c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>

typedef struct _chunk_t {
    size_t size;
    bool is_free;

    struct _chunk_t *fd;
} chunk_t;

typedef struct _free_list_t {
    chunk_t *head, *tail;
} free_list_t;

#define align_to_word(size) (size + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1)

#define LASSERT(condition, msg) \
    do \
    { \
        if(!(condition)) \
        { \
            fprintf(stderr, \
		"[*] Process terminated! 'LASSERT()' failed!\n" \
		"Condition\t: {%s}\n" \
	        "Function\t: {%s}\n" \
		"Failed in file\t: {%s}\n" \
	        "At line \t: {%d}\n", #condition, __func__, __FILE__, __LINE__); \
            fprintf(stderr, "Debug log: %s\n", msg); \
            exit(1); \
        } \
    } while(0) \

free_list_t free_list = { .head = NULL, .tail = NULL };

static __always_inline chunk_t *first_fit(size_t size) {
    LASSERT(size, "Invalid size!");
 
    for(chunk_t *tmp = free_list.head; tmp; tmp = tmp->fd) {
        if(tmp->size >= size && tmp->is_free) {
            return tmp;
        }
    }
    
    return NULL;
}

static __always_inline void insert_chunk(chunk_t *chunk) {
    LASSERT(chunk, "Chunk is NULL");

    if(!free_list.head) {
        free_list.head = free_list.tail = chunk;
        return;
    }

    free_list.tail->fd = chunk;
    free_list.tail = free_list.tail->fd;
}

chunk_t *request_space(size_t size) {    
    void *memory = mmap(NULL, sizeof(chunk_t) + size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    LASSERT(memory != MAP_FAILED, "Failure to map memory!");
    
    chunk_t *chunk = (chunk_t *) memory;
    chunk->size = size;
    chunk->is_free = false;
    chunk->fd = NULL;

    insert_chunk(chunk);

    return chunk;
}

void *dsma_alloc(size_t size) {
    LASSERT(size, "Invalid allocation size!");

    size = align_to_word(size);

    chunk_t *chunk = first_fit(size);

    if(!chunk) {
        chunk = request_space(size);

        if(!chunk) {
            return NULL;
        }
    }

    chunk->is_free = false;

    return (chunk + 1);
}

void dsma_free(void *addr) {
    LASSERT(addr, "Trying to free invalid address!");
    
    chunk_t *header = (chunk_t *)addr - 1;
    header->is_free = true;
}
```

Example usage:
```c

int main(void) {
    int *ptr1 = dsma_alloc(16);
    *ptr1 = 5;

    int *ptr2 = dsma_alloc(100);
    *ptr2 = 10;

    int *ptr3 = dsma_alloc(8);
    *ptr3 = 20;

    int *ptr4 = dsma_alloc(4);
    *ptr4 = 30;

     printf(
        "ptr1 address => %p\nptr1 value => %d\n\n"
        "ptr2 address => %p\nptr2 value => %d\n\n"
        "ptr3 address => %p\nptr3 value => %d\n\n"
        "ptr4 address => %p\nptr4 value => %d\n",
        (void *) ptr1, *ptr1,
        (void *) ptr2, *ptr2,
        (void *) ptr3, *ptr3,
        (void *) ptr4, *ptr4
    );


    dsma_free(ptr1);
    dsma_free(ptr2);
    dsma_free(ptr3);
    dsma_free(ptr4);
```

## Design notes:
* Neither implementation support thread-safety.
* Generally non-optimized forms.
