/**
 * Malloc
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _metadata_t
{
    unsigned int size;
    unsigned char isUsed;

    struct _metadata_t *next_free;
    struct _metadata_t *prev_free;
} metadata_t;

metadata_t *head_free = NULL;
metadata_t *tail_free = NULL;
void *startOfHeap = NULL;

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size)
{
    // implement calloc:

    void *mem = malloc(num * size);
    memset(mem, 0, num * size);
    return mem;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size)
{
    // implement malloc:
    // printf("After malloc\n");

    // curMeta = (void *)curMeta + sizeof(metadata_t) + curMeta->size;

    if (startOfHeap == NULL)
        startOfHeap = sbrk(0);

    if (head_free != NULL)
    {
        metadata_t *curMeta = head_free;
        while (curMeta != NULL)
        {
            if (curMeta->size == size || ((curMeta->size > size) && ((curMeta->size - size) < sizeof(metadata_t))))
            {
                curMeta->isUsed = 1;

                if (curMeta == tail_free)
                {
                    tail_free = curMeta->prev_free;
                }
                else
                {
                    curMeta->next_free->prev_free = curMeta->prev_free;
                }
                if (curMeta == head_free)
                {
                    head_free = curMeta->next_free;
                }
                else
                {
                    curMeta->prev_free->next_free = curMeta->next_free;
                }

                curMeta->next_free = NULL;
                curMeta->prev_free = NULL;

                // printHeap();
                // printFree();
                return (void *)curMeta + sizeof(metadata_t);
            }
            if (curMeta->size > size + sizeof(metadata_t))
            {
                split(curMeta, (int)size);
                // printf("After Split\n");
                //  printHeap();
                //  printFree();
                return (void *)curMeta + sizeof(metadata_t);
            }
            curMeta = curMeta->next_free;
        }
    }

    metadata_t *meta = sbrk(sizeof(metadata_t));
    meta->size = size;
    meta->isUsed = 1;
    meta->next_free = NULL;
    meta->prev_free = NULL;

    void *ptr = sbrk(size);
    // printHeap();
    return ptr;
}

void split(metadata_t *current, int size)
{
    metadata_t *new_block = (void *)current + sizeof(metadata_t) + size;
    new_block->isUsed = 0;
    new_block->size = current->size - size - sizeof(metadata_t);

    new_block->next_free = current->next_free;
    new_block->prev_free = current->prev_free;

    if (current == tail_free)
    {
        tail_free = new_block;
    }
    else
    {
        current->next_free->prev_free = new_block;
    }

    if (current == head_free)
    {
        head_free = new_block;
    }
    else
    {
        current->prev_free->next_free = new_block;
    }

    if (new_block != tail_free && new_block->next_free == ((void *)new_block + sizeof(metadata_t) + new_block->size))
    {
        coalesce(new_block);
    }

    current->next_free = NULL;
    current->prev_free = NULL;
    current->isUsed = 1;
    current->size = size;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr)
{
    // implement free
    // printf("After Free\n");

    // Find the metadata located immediately before ptr
    metadata_t *meta = ptr - sizeof(metadata_t);

    // Mark the allocation as free
    meta->isUsed = 0;

    if (head_free == NULL)
    {
        head_free = meta;
        tail_free = meta;
        // printFree();
        return;
    }
    metadata_t *curr = head_free;
    __intptr_t meta_point = (__intptr_t)meta;
    int found = 0;
    while (curr != NULL)
    {
        __intptr_t curr_point = (__intptr_t)curr;
        if (curr_point > meta_point)
        {
            meta->prev_free = curr->prev_free;
            if (curr == head_free)
            {
                head_free = meta;
            }
            else
            {
                curr->prev_free->next_free = meta;
            }
            curr->prev_free = meta;
            meta->next_free = curr;

            found = 1;
            break;
        }
        curr = curr->next_free;
    }
    if (!found)
    {
        tail_free->next_free = meta;
        meta->prev_free = tail_free;
        tail_free = meta;
    }

    if (meta != tail_free && meta->next_free == ((void *)meta + sizeof(metadata_t) + meta->size))
    {
        coalesce(meta);
    }
    if (meta != head_free && ((void *)meta->prev_free + sizeof(metadata_t) + meta->prev_free->size) == meta)
    {
        coalesce(meta->prev_free);
    }
    // printFree();
}

void coalesce(metadata_t *curr)
{
    // Merge on free linked list
    // printf("After coalescing\n");
    curr->size = curr->size + sizeof(metadata_t) + curr->next_free->size;

    if (curr->next_free == tail_free)
    {
        tail_free = curr;
    }
    else
    {
        curr->next_free->next_free->prev_free = curr;
    }

    metadata_t *temp = curr->next_free;
    curr->next_free = curr->next_free->next_free;

    temp->next_free = NULL;
    temp->prev_free = NULL;
    temp->size = 0;

    // printHeap();
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size)
{
    // implement realloc:
    // printf("Realloc reached\n");
    if (!ptr)
        return malloc(size);

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    metadata_t *meta = ptr - sizeof(metadata_t);
    if (meta->size == size)
    {
        return ptr;
    }
    if (meta->size >= size + sizeof(metadata_t))
    {
        metadata_t *new_block = (void *)meta + sizeof(metadata_t) + size;
        new_block->isUsed = 0;
        new_block->size = meta->size - size - sizeof(metadata_t);
        new_block->next_free = NULL;
        new_block->prev_free = NULL;

        if (head_free == NULL)
        {
            head_free = new_block;
            tail_free = new_block;
        }
        else
        {
            metadata_t *curr = (void *)new_block + sizeof(metadata_t) + new_block->size;
            void *endOfHeap = sbrk(0);
            int found = 0;
            while ((void *)curr < endOfHeap)
            {
                if (!curr->isUsed)
                {
                    new_block->prev_free = curr->prev_free;
                    if (curr == head_free)
                    {
                        head_free = new_block;
                    }
                    else
                    {
                        curr->prev_free->next_free = new_block;
                    }
                    curr->prev_free = new_block;
                    new_block->next_free = curr;

                    found = 1;
                    break;
                }
                curr = (void *)curr + sizeof(metadata_t) + curr->size;
            }
            if (!found)
            {
                tail_free->next_free = new_block;
                new_block->prev_free = tail_free;
                tail_free = new_block;
            }
            // if (new_block != tail_free && new_block->next_free == ((void *)new_block + sizeof(metadata_t) + new_block->size)) {
            //   coalesce(new_block);
            // }
        }
        meta->size = size;
        // printHeap();
        // printFree();
        return ptr;
    }
    else
    {
        void *new_mem = malloc(size);
        memcpy(new_mem, ptr, meta->size);
        free(ptr);
        // printHeap();
        // printFree();
        return new_mem;
    }
}

void printHeap()
{
    // Print out data about each metadata chunk:
    metadata_t *curMeta = startOfHeap;
    void *endOfHeap = sbrk(0);
    printf("-- Start of Heap (%p) --\n", startOfHeap);
    while (curMeta < sbrk(0))
    {
        printf("metadata for memory %p: (%p, size=%d, isUsed=%d)\n", (void *)curMeta + sizeof(metadata_t), curMeta, curMeta->size, curMeta->isUsed);
        curMeta = (void *)curMeta + sizeof(metadata_t) + curMeta->size;
    }
    printf("-- End of Heap (%p) --\n\n", endOfHeap);
}

void printFree()
{
    // Print out data about each metadata chunk:
    metadata_t *curMeta = head_free;
    printf("-- Start of Free LL (%p) --\n", head_free);
    while (curMeta != NULL)
    {
        printf("metadata for memory %p: (%p, size=%d, isUsed=%d)\n", (void *)curMeta + sizeof(metadata_t), curMeta, curMeta->size, curMeta->isUsed);
        curMeta = curMeta->next_free;
    }
    printf("-- End of Free LL (%p) --\n\n", tail_free);
}