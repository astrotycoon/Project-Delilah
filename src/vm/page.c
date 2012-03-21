#include "vm/page.h"
#include <debug.h>
#include <stddef.h>
//TODO - remove stdio
#include <stdio.h>
#include <string.h>
#include "devices/block.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "vm/frame.h"
#include "vm/swap.h"

/* Loads a page from the file system into memory */
void page_filesys_load (struct page *upage, void *kpage);

//TODO - comment
static bool page_load_from_mapped_file (struct page *upage, void *fault_addr);

bool
page_load (struct page *upage, void *fault_addr)
{

  //if (pagedir_get_page(thread_current()->pagedir,upage->uaddr)==NULL)
  //  printf("already mapped :(\n");

  /* Load the page into memory again.*/
  if (upage->saddr != -1)
    {
      void *kpage = palloc_get_page (PAL_USER);
      /* Load from swap. */
      install_page (upage->uaddr, kpage, upage->write);
      swap_read_page (upage);
    }
  else
    {
      /* Load from a file. */
      if (upage->file == NULL)
        {
          /* Memory-mapped file. */
          return page_load_from_mapped_file (upage, fault_addr);
        }

      /* Get a page of memory. */
      uint8_t *kpage = palloc_get_page (PAL_USER);
      if (kpage == NULL)
        {
          frame_evict ();
          //printf ("bad things happened1\n");
          kpage = palloc_get_page (PAL_USER);
          //thread_exit ();
        }

      /* Load this page. */
      if (file_read_at (upage->file, kpage, upage->file_read_bytes,
              upage->file_start_pos)
            != (int) upage->file_read_bytes)
        {
          palloc_free_page (kpage);
          printf ("bad things happened2\n");
          thread_exit ();
        }

      memset (kpage + upage->file_read_bytes, 0, PGSIZE - upage->file_read_bytes);
      /* Add the page to the process's address space. */
      if (!install_page (upage->uaddr, kpage, upage->write)) 
        {
          palloc_free_page (kpage);
          printf ("bad things happened3\n");
          thread_exit ();
        } 
    }
  return true;
}

static bool
page_load_from_mapped_file (struct page *upage, void *fault_addr)
{
  void *orig_fault_addr = fault_addr;

  struct mapped_file *mapped_file = thread_get_mapped_file (orig_fault_addr);
  if (mapped_file == NULL)
    return false;

  void *in_file_addr = (void *) (orig_fault_addr - mapped_file->addr);
  uint8_t *buffer = palloc_get_page (PAL_USER);
  if (buffer == NULL)
    {
      printf ("damn\n");
      return false;
    }
  int bytes_read = file_read_at (mapped_file->file, buffer, PGSIZE,
      (int) pg_round_down (in_file_addr));

  memset (buffer + bytes_read, 0, PGSIZE - bytes_read);

  /* Add the page to the process's address space. */
  if (!install_page (upage->uaddr, buffer, upage->write)) 
    {
      palloc_free_page (buffer);
      printf ("bad things happened3\n");
      return false;
    }

  return true;
}

void
page_create (struct frame *frame)
{
  /* Create the page struct */
  struct page *page = malloc (sizeof (struct page));
  page->saddr = -1;
  page->uaddr = frame->uaddr;
  page->write = frame->write;
 
  /* Write the page to swap or filesys */

  /* Destroy the frame */
  uninstall_page (frame->addr);
  free (frame);
}

void
page_write (struct page *upage, struct frame *frame)
{ 
  struct hash_elem *e = hash_insert (&frame->owner->sup_page_table, &upage->hash_elem);
  
  //if (e == NULL) 
    //printf ("inserted addr: %p\n", upage->uaddr);
  //else
    //printf ("arse: %p", upage->uaddr);

  
    //if (upage->saddr != -1)
  
  upage->saddr = swap_write_page (upage);
  
  //printf ("it worked");
  //else
    //PANIC ("AAAEFFFFH");
}

void
page_filesys_load (struct page *upage UNUSED, void *kpage UNUSED)
{
  //TODO - page_filesys_load
}

struct page *
page_lookup (struct hash *page_table, void *uaddr)
{
  struct page p;
  struct hash_elem *e;

  p.uaddr = uaddr;
  e = hash_find (page_table, &p.hash_elem);
  return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
}

unsigned
page_hash_func (const struct hash_elem *e, void *aux UNUSED)
{
  const struct page *p = hash_entry (e, struct page, hash_elem);
  return hash_bytes (&p->uaddr, sizeof &p->uaddr);
}

bool
page_less_func (const struct hash_elem *a, const struct hash_elem *b,
                void *aux UNUSED)
{
  const struct page *pa = hash_entry (a, struct page, hash_elem);
  const struct page *pb = hash_entry (b, struct page, hash_elem);
  return pa->uaddr < pb->uaddr;
}
