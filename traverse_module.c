#include <stddef.h>
//#include <linux/vmalloc.h>
#include "traverse_module.h"
//#include <linux/module.h>
//MODULE_LICENSE("GPL");





int inside_the_page(mchunkptr p ,size_t VpageNO) 
{
  // return true if the chunk is inside the virtual page
  if (((int64_t)p >> 12) == VpageNO)
  {
    return 1;
  } else 
  {
    return 0;
  }
}

void add_free_chunk(free_chunk_info_ptr *free_chunk_info_head_ptr, free_chunk_info_ptr *free_chunk_info_current_ptr, mchunkptr *p_ptr, size_t *num)
{
  if (free_chunk_info_head_ptr == NULL)
  {
    //*free_chunk_info_head_ptr = *free_chunk_info_current_ptr = (free_chunk_info_ptr)kmalloc(sizeof(free_chunk_info_t), GFP_KERNEL);
    *free_chunk_info_head_ptr = *free_chunk_info_current_ptr = (free_chunk_info_ptr)malloc(sizeof(free_chunk_info_t));
  } else
  {
    //(*free_chunk_info_current_ptr)->next = (free_chunk_info_ptr)kmalloc(sizeof(free_chunk_info_t), GFP_KERNEL);
    (*free_chunk_info_current_ptr)->next = (free_chunk_info_ptr)malloc(sizeof(free_chunk_info_t));
    (*free_chunk_info_current_ptr) = (*free_chunk_info_current_ptr)->next;
  }
  (*free_chunk_info_current_ptr)->start = *p_ptr;
  (*free_chunk_info_current_ptr)->len = chunksize(*p_ptr); // size of the data chunk (including header and mem)
  (*free_chunk_info_current_ptr)->next = NULL;
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

free_chunk_info_t* traverse (void* arena_start_ptr, size_t VpageNO, size_t* len) 
{
  mstate av = (mstate)arena_start_ptr;
  free_chunk_info_ptr free_chunk_info_head = NULL;
  free_chunk_info_ptr free_chunk_info_current = NULL;
  mchunkptr p;
  int i;

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
  mchunkptr b;
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
  mchunkptr top_addr = av->top;
  if (inside_the_page(top_addr, VpageNO))
  {
    if (inside_the_page(p, VpageNO))
    {
      add_free_chunk(&free_chunk_info_head, &free_chunk_info_current, &p, len);
    }
  }
  return free_chunk_info_head;
}
