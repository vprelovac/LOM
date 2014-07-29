/*
 * Memory data structure.
 */

#if defined(ADV_MEMORY)

void init_memory(void);
void *acquire_memory(int, int, const char *, const int);

struct mem_data1 {
    int line;
    int size;
    void *ptr;
    const char *func;
    struct mem_data1 *next;
};

#define get_memory(number, size)	acquire_memory((number), (size), __FUNCTION__, __LINE__)

#define CREATE(result, type, number)					\
  if (!((result) = (type *) get_memory((number), sizeof(type)))) {	\
    perror("malloc failure");						\
    abort();								\
  }									\

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
                { perror("realloc failure"); abort(); } } while(0)

#endif
