# Memory-allocators
Educational repository to briefly go over practical implementations of memory allocators and theoretical background of them.

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
Mmap is ideal for really large chunk alloctations that are around 4096 bytes worth of allocation space.

# What's the heap?
We threw the word "heap" around a lot, but actually is the heap? The heap is a sesction of memory in our process that is constructed through the utilization of those functions / system calls we mentioned. Values stored on the heap are loaded at runtime.

# Types of memory allocators:
So lets get into what they actually look in code and their types.

## Linear allocator / bump allocators / arena allocators:
Linear allocators are allocators that allocate a chunk of memory deteremined by a given size that you index into with a pointer. You generally keep track of 3 different points. A pointer to the start of the allocated buffer, a current pointer that keeps track of the current pointer we have into the memory space. And an end pointer to make sure we don't write the current pointer past the memory space we defined.

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
  * First fit: 
  * Best fit:
  * Worst fit:

### Deallocation:
### Pros:
### Cons:

### Notes:
Tons of modern memory allocators such as ``ptmalloc`` and ``dlmalloc`` use this type of allocator. Some allocators implemnet binning to store certain lengths of chunk ranges for quick access of free list. Some implement seperate linked lists for different sizes for even faster access to free chunks. Some implement a binary search tree that orders based on size or chunk address. A lot of allocators implement block splitting from recently fetched searching operations. A common optimization that ``free`` would implement would be block coalescing, where we conbaine adjencet free chunks into a big free chunk and mark it as new.There are a ton of optimizations in this field.

## Pool allocators:
### Allocation:
### Deallocation:
### Pros:
### Cons:

# Fragmentation:

# Why would we write our own?
Why would we wriute our own? Given memory allocator interfaces such as: ``malloc / free()``

# Implementations:
