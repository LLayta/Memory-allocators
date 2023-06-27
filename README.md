# Memory-allocators
Educational repository to briefly go over practical implementations of memory allocators and theoretical background of them.

# Why?
Why do we care about memory allocators? Memory allocators are an interface that allow us to preform *dynamic memory allocation* which is when we store long lived values in a programs' memory that is **not** the stack which is automatically sized at compile-time, the size of dynamically allocated memory is deteremined at run-time. 

# How?
Memory allocators (typically) utilize a part of processes' memory space called the "heap". We can dynamically allocate data onto the heap in 2 main ways on linux.
  * mmap system call: Maps a chunk of memory that is utilized by our program that we store in between the data segment (.rodata, .text and what not) and the stack. See figure.1 for visualization.
  * sbrk function: Alters the program break to allocate space directly above the data segment. (Program break is a pointer to the top of the data segment on the first allocation but after allocations it'll be a pointer to the top of the heap). See figure.1 for visualization.

# Addressing space visualization:
![Addressing space](imgs/figure1.png)

# What's the heap?

# Types of memory allocators:

# Why would we write our own?

# Implementations:
