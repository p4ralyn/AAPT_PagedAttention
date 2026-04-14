// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// --- AAPT CoW: Reference Counter Array ---
int ref_count[PHYSTOP / PGSIZE];

// Helper to increment the reference count safely
void incref(uint64 pa) {
  int pn = pa / PGSIZE;
  acquire(&kmem.lock);
  ref_count[pn]++;
  release(&kmem.lock);
}
// -----------------------------------------

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // --- AAPT CoW: Check Reference Count ---
  int pn = (uint64)pa / PGSIZE;
  acquire(&kmem.lock);
  if(ref_count[pn] > 1) {
    ref_count[pn]--;       // Just drop the count, someone else is using it!
    release(&kmem.lock);
    return;
  }
  ref_count[pn] = 0;       // Count is 0, actually free the memory
  release(&kmem.lock);
  // ---------------------------------------

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) {
    kmem.freelist = r->next;
    // --- AAPT CoW: Initialize Ref Count ---
    int pn = (uint64)r / PGSIZE;
    ref_count[pn] = 1;
    // --------------------------------------
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

int krefcount(uint64 pa) { //helper for cow_handler
  return ref_count[pa / PGSIZE];
}

int 
count_free_pages(void) {
  struct run *r;
  int count = 0;
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    count++;
    r = r->next;
  }
  release(&kmem.lock);
  return count;
}