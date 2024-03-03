#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>


#define ALIGNMENT 16  
#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))

#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

#define PADDED_SIZE(x) ((x) + GET_PAD(x))



struct block {
    int size;            
    int in_use;          
    struct block *next;
};



void *heap = NULL;
struct block *head = NULL;

void initialize_alloc(){

    heap = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

    int data_size = 1024 - PADDED_SIZE(sizeof(struct block));

    head = (struct block *)heap;
    head->size = data_size;
    head->in_use = 0;
    head->next = NULL;
}



void *myalloc(int size){

    if (heap == NULL){
        initialize_alloc();
    }

    size_t padded_size = PADDED_SIZE(size);

    struct block *current = head;

    while (current != NULL) {

        if (!current->in_use && (size_t)current->size >= padded_size) {
            size_t unused_space = current->size - padded_size - PADDED_SIZE(sizeof(struct block));

            if (unused_space >= sizeof(struct block)) {
                struct block *new_block = (struct block *)PTR_OFFSET(current, padded_size);

                new_block->size =  unused_space;
                new_block->in_use = 0;
                new_block->next = current->next;

                current->size = padded_size;
                current->in_use = 1;
                current->next = new_block;

            } else {
                current->in_use = 1;

            }
                return PTR_OFFSET(current, sizeof(struct block));

        }

        current = current->next;
    }

    return NULL;

}

void coalesce_space(struct block *head){


    if (head == NULL || head->next == NULL){
        return;
    }


    while (head->next != NULL) {

    if (!head->in_use && !head->next->in_use){
        head->size += head->next->size + PADDED_SIZE(sizeof(struct block));
        head->next = head->next->next;

        } else {
            head = head->next;
        }
    }
}



void myfree(void *ptr) {

    struct block *current = head;

    while (current != NULL){

        void *block_pointer = PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)));

        if (block_pointer == ptr){

            current->in_use = 0;
            coalesce_space(head);

            return;

        }

        current = current->next;
    }


}



void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        // Uncomment the following line if you want to see the pointer values
        //printf("[%p:%d,%s]", b, b->size, b->in_use? "used": "free");
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}



int main(void) {


    void *p, *q, *r, *s;

    p = myalloc(10); print_data();
    q = myalloc(20); print_data();
    r = myalloc(30); print_data();
    s = myalloc(40); print_data();

    myfree(q); print_data();
    myfree(p); print_data();
    myfree(s); print_data();
    myfree(r); print_data();


}