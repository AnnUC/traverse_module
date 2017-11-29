#include <stddef.h>
//#include <linux/vmalloc.h>
#include "traverse_module.h"
//#include <linux/module.h>
//MODULE_LICENSE("GPL");


#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

/* The corresponding word size */
#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))

#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT       (2 *SIZE_SZ)
#endif

 /* The corresponding bit mask value */
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)

#ifndef offsetof
# define offsetof(type,ident) ((size_t)&(((type*)0)->ident))
#endif

/* The smallest possible chunk */
#define MIN_CHUNK_SIZE        (offsetof(struct malloc_chunk, fd_nextsize))

/* The smallest size we can malloc is an aligned minimal chunk */

#define MINSIZE  \
  (unsigned long)(((MIN_CHUNK_SIZE+MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK))

#define request2size(req)                                         \
  (((req) + SIZE_SZ + MALLOC_ALIGN_MASK < MINSIZE)  ?             \
   MINSIZE :                                                      \
   ((req) + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)

#define fastbin_index(sz) \
  ((((unsigned int) (sz)) >> (SIZE_SZ == 8 ? 4 : 3)) - 2)

typedef struct malloc_chunk *mbinptr;
#define bin_at(m, i) \
  (mbinptr) (((char *) &((m)->bins[((i) - 1) * 2]))           \
             - offsetof (struct malloc_chunk, fd))  

/* The maximum fastbin request size we support */
#define MAX_FAST_SIZE     (80 * SIZE_SZ / 4)
#define NFASTBINS  (fastbin_index (request2size (MAX_FAST_SIZE)) + 1)

#define fastbin(ar_ptr, idx) ((ar_ptr)->fastbinsY[idx])

#define SIZE_BITS (PREV_INUSE | IS_MMAPPED | NON_MAIN_ARENA)

/* Get size, ignoring use bits */
#define chunksize(p)         ((p)->size & ~(SIZE_BITS))


bool inside_the_page(mchunkptr p ,size_t VpageNO) 
{
  // return true if the chunk is inside the virtual page
  if ((p >> 12) == VpageNO) //double check???
  {
    return true;
  } else 
  {
    return false;
  }
}

void add_free_chunk(free_chunk_info_ptr *free_chunk_info_head_ptr, free_chunk_info_ptr *free_chunk_info_current_ptr, mchunkptr *p_ptr, size_t *num)
{
  if (free_chunk_info_head_ptr == NULL)
  {
    *free_chunk_info_head_ptr = *free_chunk_info_current_ptr = (free_chunk_info_ptr)malloc(sizeof(free_chunk_info), GFP_KERNEL);
  } else
  {
    (*free_chunk_info_current_ptr)->next = (free_chunk_info_ptr)malloc(sizeof(free_chunk_info), GFP_KERNEL);
    (*free_chunk_info_current_ptr) = (*free_chunk_info_current_ptr)->next;
  }
  (*free_chunk_info_current_ptr)->chunk_start_ptr = *p_ptr;
  (*free_chunk_info_current_ptr)->len = chunksize(*p_ptr); // size of the data chunk (including header and mem)
  (*free_chunk_info_current_ptr)->next = NULL；
  (*num)++;
}

/* 
   get the free chunk information in current virtual page, VpageNO.
   input: 
    arena_start_ptr: pointer to the arena containing the virtual page VpageNO.
    VpageNO: Virtual Page NO
    len: number of the free chunks 
   output:
    pointer to the free chunk list 
 */
free_chunk_info* traverse_func (void* arena_start_ptr, size_t VpageNO, size_t* len) 
{

}
free_chunk_info* traverse (void* arena_start_ptr, size_t VpageNO, size_t* len) 
{
  mstate av = (mstate)arena_start_ptr;
  free_chunk_info_ptr free_chunk_info_head = NULL;
  free_chunk_info_ptr free_chunk_info_current = NULL;
  mchunkptr p;

  // traverse fast bins 
  for (i = 0; i < NFASTBINS; ++i) { 
    for (p = fastbin (av, i); p != 0; p = p->fd) 
    {
      if (inside_the_page(p, VpageNO))
      {
        add_free_chunk(&free_chunk_info_head, &free_chunk_info_current, &p, len);
      }
    }
  }

  // traverse regular bins 
  malloc_chunk* b;
  for (i = 1; i < NBINS; ++i) {
    b = bin_at (av, i); //
    for (p = (b)->bk; p != b; p = p->bk) { 
      //address: p  size: chunksize(p)
      if (inside_the_page(p, VpageNO))
      {
        add_free_chunk(&free_chunk_info_head, &free_chunk_info_current, &p, len);
      }
    }
  }

  //get top chunk
  malloc_chunk* top_addr = av->top;
  if (inside_the_page(top_addr, VpageNO))
  {
    if (inside_the_page(p, VpageNO))
    {
      add_free_chunk(&free_chunk_info_head, &free_chunk_info_current, &p, len);
    }
  }
  return free_chunk_info_head;
}
