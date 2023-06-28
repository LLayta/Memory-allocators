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
}
