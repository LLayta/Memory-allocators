#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define align_to_word(size) (size + (sizeof(void *) - 1)) & ~(sizeof(void *) - 1)

typedef struct _Allocator_t {
    char *start, *end, *curr;
    size_t capacity;
} Allocator_t;

Allocator_t *new_allocator(size_t capacity) {
    if(!capacity) {
        return NULL;
    }

    Allocator_t *alloc = calloc(1, sizeof(*alloc));
    if(!alloc) {
        return NULL;
    }
    
    alloc->start = malloc(capacity);
    if(!alloc->start) {
        return NULL;
    }

    alloc->capacity = capacity;
    alloc->curr = alloc->start;
    alloc->end = alloc->start + capacity;

    return alloc;
}

void *allocator_alloc(Allocator_t *alloc, size_t offset) {
    if(!alloc || !offset) {
        return NULL;
    }

    offset = align_to_word(offset);

    if(offset > alloc->capacity) {
        return NULL;
    }
    
    if(alloc->curr + offset <= alloc->end) {
        char *tmp = alloc->curr;
        alloc->curr += offset;
        
        return tmp;
    }

    return NULL;
}

void allocator_reset(Allocator_t *alloc) {
    if(!alloc || !alloc->start) {
        return;
    }

    free(alloc->start);
    alloc->curr  = NULL;
    alloc->end   = NULL;
    alloc->start = NULL;
}

void print_state(Allocator_t *alloc) {
    if(!alloc) {
        return;
    }

    printf("Curr: %p\n"
           "Start: %p\n"
           "End: %p\n", alloc->curr, alloc->start, alloc->end);
}

int main(void) {
    Allocator_t *alloc = new_allocator(100);
    
    int *ptr1 = allocator_alloc(alloc, 5 * sizeof(int));
    for(int i = 0; i < 5; ++i) {
        ptr1[i] = i;
        printf("Ptr1[%d] => %d\n", i, ptr1[i]);
    }

    putchar('\n');

    char char_arr[] = { 'a', 'b', 'c', 'e', 'd'},
    *ptr2 = allocator_alloc(alloc, 5);
    
    for(int i = 0; i < 5; ++i) {
        ptr2[i] = char_arr[i];
        printf("Ptr2[%d] => %c\n", i, ptr2[i]);
    }

    putchar('\n');

    double double_arr[] = { 10.00, 20.00, 30.00, 40.00, 50.00 },
    *ptr3 = allocator_alloc(alloc, 5 * sizeof(int));

    for(int i = 0; i < 5; ++i) {
        ptr3[i] = double_arr[i];
        printf("Ptr3[%d] => %lf\n", i, ptr3[i]);
    }

    putchar('\n');

    print_state(alloc);
    allocator_reset(alloc);
    print_state(alloc);
}
