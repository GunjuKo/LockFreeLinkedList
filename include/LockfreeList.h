#ifndef LOCK_FREE_LIST
#define LOCK_FREE_LIST

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#define BITWISE_0 0x7fffffffffffffff
#define BITWISE_1 0x8000000000000000


typedef struct lock {
    unsigned long table_id;
    unsigned long record_id;
    int lock_state;

    struct lock* next;
} lock_t;


typedef struct list{
    lock_t head;
    lock_t* tail;
} list_t;

void init_list_t(list_t* list)
{
    assert(list != NULL);
    list->head.next = NULL;
    list->tail      = &(list->head);
}

lock_t* getNextReference(lock_t* node);
bool getLogicalRemovedBit(lock_t* node);

void print_list(list_t* list)
{
    lock_t* head = list->head.next;
    int count = 0;
    while(head != NULL)
    {
//        if(getLogicalRemovedBit(head) == false){
            printf("%d\n", head->record_id);
            count++;
//        }
        head = getNextReference(head);
    }

    printf("count : [%d]\n",count); 
}

lock_t* getNextReference(lock_t* node)
{
    return (lock_t*)((intptr_t)(node->next)&BITWISE_0);
}

bool getLogicalRemovedBit(lock_t* node)
{
    if(((intptr_t)(node->next) & BITWISE_1) == 0)
        return false;
    else
        return true;
}

bool list_append(list_t* list, lock_t* new_elem)
{
    bool snip;

    while(true){
        lock_t* old_tail = __sync_lock_test_and_set(&(list->tail), 
            new_elem);
        
        snip = __sync_bool_compare_and_swap(&(old_tail->next),
                (lock_t* )((intptr_t)NULL & BITWISE_1), new_elem);

        if(snip){
            break;
        }
    }
}

void list_remove(list_t* list, lock_t* remove_elem)
{
    bool snip;

    /* logically remove */
    __sync_lock_test_and_set(&(remove_elem->next), 
           (lock_t*)((intptr_t)(remove_elem->next)|BITWISE_1));

    lock_t* curr = list->head.next;
    lock_t* prev = &(list->head);
    lock_t* succ;
    
    while(curr != NULL)
    {
        succ = getNextReference(curr);
        if(getLogicalRemovedBit(curr) == true){
            printf("remove gogo!\n");
  
            snip = __sync_bool_compare_and_swap(&(prev->next),
                    (lock_t*)((intptr_t)curr & BITWISE_0),
                    (lock_t*)((intptr_t)succ & BITWISE_0));
            
            if(!snip){
                break;
            }

//            __sync_bool_compare_and_swap(&(list->tail),
//                    getNextReference(prev), prev);

            printf("remove success!\n");
        }
        prev = curr;
        curr = getNextReference(curr);
    }

}

#endif
