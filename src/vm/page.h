#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <stdbool.h>
#include <stdint.h>
#include "devices/block.h"
#include "vm/frame.h"
//#include "filesys/off_t.h"


struct page
  {
   struct hash_elem hash_elem; /* Hash element for a supplemental page table */
   uint8_t *uaddr;             /* Address of the page in user virtual memory.*/
   int saddr;                  /* Index of the swap slot */
   const char *name;           /* Name of the page if stored in filesys. */
   bool write;                 /* Boolean to indicate read/write permissions.*/
  };

/* Called when there is a page fault to load the relevant page back into
   memory. */
void page_load (struct page *upage);

struct page * page_create (struct frame *frame);

/* Writes a page to swap, or does nothing in the case of it being
   an unmodified file. */
void page_write (uint32_t *pd, struct page *upage);

unsigned page_hash_func (const struct hash_elem *e, void *aux);

bool page_less_func (const struct hash_elem *a, const struct hash_elem *b,
                      void *aux);

/* Looks up a page in PAGE_TABLE specified by the virtual address
   in UADDR. Returns NULL if no page is found. */
struct page * page_lookup (struct hash *page_table, void *uaddr);

#endif
