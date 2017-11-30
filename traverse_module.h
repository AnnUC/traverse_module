#ifndef TRAVERSE_MODULE_H
#define TRAVERSE_MODULE_H
#include <usr-spinlock.h>

#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

struct malloc_chunk {

  INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* Only used for large blocks: pointer to next larger size.  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};
typedef struct malloc_chunk* mchunkptr;

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

typedef struct malloc_chunk *mfastbinptr;

struct malloc_state
{
  /* Serialize access.  */
  usr_spinlock mutex;

  /* Flags (formerly in max_fast).  */
  int flags;

  /* Fastbins */
  mfastbinptr fastbinsY[NFASTBINS];

  /* Base of the topmost chunk -- not otherwise kept in a bin */
  mchunkptr top;

  /* The remainder from the most recent split of a small request */
  mchunkptr last_remainder;

  /* Normal bins packed as described above */
  mchunkptr bins[NBINS * 2 - 2];

  /* Bitmap of bins */
  unsigned int binmap[BINMAPSIZE];

  /* Linked list */
  struct malloc_state *next;

  /* Linked list for free arenas.  Access to this field is serialized
     by free_list_lock in arena.c.  */
  struct malloc_state *next_free;

  /* Number of threads attached to this arena.  0 if the arena is on
     the free list.  Access to this field is serialized by
     free_list_lock in arena.c.  */
  INTERNAL_SIZE_T attached_threads;

  /* Memory allocated from the system in this arena.  */
  INTERNAL_SIZE_T system_mem;
  INTERNAL_SIZE_T max_system_mem;

  /* updated in OCT 23 2017
   author: Xiaoan Ding
   add update_faulty_address flag, faulty_address_info_buf, disabled_chunk_info: START 
   update_faulty_address == 1 denotes an update in faulty address info*/
   // int update_faulty_address;
   // faulty_address_info_t faulty_address_info_head;
   // disabled_chunk_info_t disabled_chunk_info_head;
  /* add update_faulty_address flag: END */
};
typedef struct malloc_state *mstate;

typedef struct free_chunk_info {
  
  void* start; // pointer to the beginning of a free chunk, containing the (header+data)
  
  size_t len; // size of the free chunk
} free_chunk_info_t;
typedef struct free_chunk_info *free_chunk_info_ptr;



free_chunk_info* traverse (void* arena_start_ptr, size_t VpageNO, size_t* len);
free_chunk_info* traverse_func (void* arena_start_ptr, size_t VpageNO, size_t* len);
#endif
