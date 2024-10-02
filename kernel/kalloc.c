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

// per-CPU freelist
struct {
  struct run *freelist;
  struct spinlock kmem_lock;
} kmemcpu[NCPU];

void
kinit()
{
  // initlock(&kmem.lock, "kmem");
  for(int i = 0; i < NCPU; i++){
    initlock(&kmemcpu[i].kmem_lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
kfree_page_cpu(int cpu, void *pa){
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  r = (struct run*)pa;

  acquire(&kmemcpu[cpu].kmem_lock);
  r->next = kmemcpu[cpu].freelist;  // add the page to the freelist
  kmemcpu[cpu].freelist = r;
  release(&kmemcpu[cpu].kmem_lock);
}

void
freerange(void *pa_start, void *pa_end)
{
  // char *p;
  // p = (char*)PGROUNDUP((uint64)pa_start);
  // for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
  //   kfree(p);

  // give all the memory to one CPU

  push_off();
  int cpu = cpuid();
  pop_off();
  
  // r = (struct run*)pa_start;
  // round up to the next page boundary

  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    kfree_page_cpu(cpu, p);
  }

}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  push_off();
  int cpu = cpuid();
  pop_off();

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmemcpu[cpu].kmem_lock);
  r->next = kmemcpu[cpu].freelist;
  kmemcpu[cpu].freelist = r;
  release(&kmemcpu[cpu].kmem_lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  push_off();
  int cpu = cpuid();
  pop_off();

  // acquire(&kmem.lock);
  acquire(&kmemcpu[cpu].kmem_lock);
  // r = kmem.freelist;
  r = kmemcpu[cpu].freelist;
  if(r){
    kmemcpu[cpu].freelist = r->next;
    release(&kmemcpu[cpu].kmem_lock);
  } else{
    release(&kmemcpu[cpu].kmem_lock);
    // current CPU has an empty freelist, try to steal from another CPU
    for(int i = 0; i < NCPU; i++){
      if(i == cpu) continue;
      acquire(&kmemcpu[i].kmem_lock); // acquire the lock of the other CPU
      r = kmemcpu[i].freelist;
      if(r){
        kmemcpu[i].freelist = r->next;
        release(&kmemcpu[i].kmem_lock);
        break;
      }
      release(&kmemcpu[i].kmem_lock);
    }
  }


  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
