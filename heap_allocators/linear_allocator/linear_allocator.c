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
