/*
 * malloc() implementation based on buffer code.
 */
#include "conf.h"
#include "sysdep.h"
#include "structs.h"
#include "utils.h"

#ifdef ADV_MEMORY

struct mem_data1 *memory_head;

void init_memory()
{
    memory_head = NULL;
}

void *calloc_memory(int number, int size)
{
    return ((size > 0 && number > 0) ? calloc(number, size) : NULL);
}

void *acquire_memory(int number, int size, const char *func, const int line)
{
    if (size <= 0)
        return NULL;
    else {
        struct mem_data1 *temp;
        char buf[128];

        if (!(temp = (struct mem_data1 *)calloc_memory(sizeof(struct mem_data1), 1)))
            return NULL;
        temp->next = memory_head;
        temp->func = func;
        temp->line = line;
        temp->size = size;
        if (!(temp->ptr = calloc_memory(number, size))) {
            DISPOSE(temp);
            return NULL;
        }
        //if (size*number>30)
        {
            sprintf(buf, "MEM: %s:%d allocated %d bytes.", func, line, size * number);
            log(buf);
        }
        return temp->ptr;
    }
}
#endif
