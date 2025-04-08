#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> 
#include <unistd.h>
#include <pthread.h>

typedef char ALIGN[16];

union header{
    struct{
        size_t size;
        uint8_t is_free; 
        struct meta_block_header_t *next; 
    }s;
    ALIGN stub; 
};

typedef union header block_header_t;

block_header_t *head,*tail;
pthread_mutex_t global_malloc_lock; 

block_header_t *get_free_block(size_t size)
{
    block_header_t *current = head; 

    while(current)
    {
        if(current->s.is_free && current->s.size >= size)
        {
            return current;
        }
        current = current->s.next; 
    }
}

void *d_malloc(size_t size)
{
    size_t total_size;
    void *block;
    block_header_t *header;

    if(!size)
    {
        return NULL; 
    }

    pthread_mutex_lock(&global_malloc_lock);

    header = get_free_block(size);

    /*if free space is found*/
    if(header)
    {
        header->s.is_free = 0;
        pthread_mutex_unlock(&global_malloc_lock);
        return (void*)(header +1);
    }

    /*free block is not found */

    total_size = sizeof(block_header_t) + size; 
    block = sbrk(total_size); 

    if(block == (void*)-1)
    {
        pthread_mutex_unlock(&global_malloc_lock);
        return NULL; 
    }

    header = block;
    header->s.size = size; 
    header->s.is_free = 0;
    header->s.next = NULL;

    if(!head)
    {
        head = header;
    }
    if(tail)
    {
        tail->s.next = header;
    }
    tail = header;
    pthread_mutex_unlock(&global_malloc_lock);
    
    return (void*)(header +1);    
}

void free(void *block)
{
    block_header_t *header;
    block_header_t *tmp; 

    void *programbreak; 
    
    if(!block)
    {
        return;
    }

    pthread_mutex_lock(&global_malloc_lock);
    header = (block_header_t*)block - 1;

    programbreak = sbrk(0);
    if((char*)block + header->s.size == programbreak)
    {
        if(head == tail)
        {
            head = NULL;
            tail = NULL;
        }else
        {
            tmp = head;
            while (tmp)
            {
                if(tmp->s.next == tail)
                {
                    tmp->s.next = NULL; 
                    tail = tmp;
                }
                tmp = tmp->s.next;
            }
        }



    }




}
