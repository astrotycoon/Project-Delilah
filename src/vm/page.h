#include <hash.h>
#include <stdbool.h>
#include <stdint.h>
//#include "filesys/off_t.h"


struct page
  {
   struct hash_elem hash_elem; /* Hash element for a supplemental page table */
   uint8_t *uaddr;             /* Address of the page in user virtual memory.*/
   uint32_t *saddr;            /* Address of the page in swap. */
   const char *name;           /* Name of the page if stored in filesys. */
   bool write;                 /* Boolean to indicate read/write permissions.*/
  };

/* Called when there is a page fault to load the relevant page back into
   memory. */
void page_load (uint32_t *pd, struct page *upage);

/* Writes a page to swap, or does nothing in the case of it being
   an unmodified file*/
void page_write (uint32_t *pd, struct page *upage);

/* Loads a page from the swap partition into memory */
void page_swap_load (struct page *upage, void *kpage);

/* Writes a page from memory to the swap partition */
void page_swap_write (struct page *upage);

/* Loads a page from the file system into memory */
void page_filesys_load (struct page *upage, void *kpage);

unsigned page_hash_func (const struct hash_elem *e, void *aux);

bool page_less_func (const struct hash_elem *a, const struct hash_elem *b,
                      void *aux);

struct page * page_lookup (struct hash *page_table, void *uaddr);
