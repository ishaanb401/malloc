# Malloc Implementation
This project provides a custom implementation of memory allocation functions (*malloc*, *realloc*, *calloc*, *free*) in C. The goal is to simulate the behavior of the C standard library's memory allocation functions, implementing them from scratch using the *sbrk()* system call

## Features
* malloc(size_t size): Allocates a block of memory of the requested size and returns a pointer to the beginning of the block.
* calloc(size_t num, size_t size): Allocates memory for an array of num elements, each of size bytes, and initializes the memory to zero.
* realloc(void *ptr, size_t size): Resizes a previously allocated memory block to a new size, preserving the data up to the minimum of the old and new sizes.
* free(void *ptr): Frees a previously allocated memory block, making it available for future allocations

## How to Build
1. **Clone the repository:**
   ```bash
   git clone https://github.com/ishaanb401/malloc.git
2. **Compile the program:**
   ```bash
   gcc -o malloc_test alloc.c
3. **Run the program:**
   ```bash
   ./malloc_test
