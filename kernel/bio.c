// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

struct spinlock steallock; // lock when stealing buffers

struct {
  // struct spinlock lock;
  struct buf buf[NBUF]; // each element represents a buffer that can store a block of data
  struct spinlock lock;
} bcache[BCACHE_NBUCKET];  // allow synchronized access to the buffer cache

uint hash(uint dev, uint blockno) {
  // return (dev ^ blockno) % BCACHE_NBUCKET;
  return ((dev << 27) | blockno) % BCACHE_NBUCKET;
}

void
binit(void)
{
  struct buf *b;
  initlock(&steallock, "steallock");

  for (int i = 0; i < BCACHE_NBUCKET; i++) {
    for (int j = 0; j < NBUF; j++) {
      b = &bcache[i].buf[j];
      b->timestamp = 0;
      b->refcnt = 0;
      initsleeplock(&b->lock, "buffer");
    }
    initlock(&bcache[i].lock, "bcache");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  // compute which bucket the block belongs to
  uint bucket = hash(dev, blockno);

  // // acquire(&steallock);

  // acquire the lock for the bucket

  acquire(&bcache[bucket].lock);

  // Is the block already cached?
  for (int i = 0; i < NBUF; i++) {
    b = &bcache[bucket].buf[i];
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      b->timestamp = ticks;
      release(&bcache[bucket].lock);
      acquiresleep(&b->lock); // ensure exclusive access to the buffer
      // // release(&steallock);
      return b;
    }
  }

  // not cached
  // recycle the least recently used (LRU) unused buffer

  // find the least recently used buffer
  struct buf *lru = 0;
  for (int i = 0; i < NBUF; i++) {
    b = &bcache[bucket].buf[i];
    if (b->refcnt == 0 && (lru == 0 || b->timestamp < lru->timestamp)) {
      lru = b;
    }
  }

  if (lru != 0) {
    // evict the LRU buffer
    lru->dev = dev;
    lru->blockno = blockno;
    lru->valid = 0;
    lru->refcnt = 1;
    lru->timestamp = ticks;
    acquiresleep(&lru->lock);
    release(&bcache[bucket].lock);
  } else {
    // no free buffers
    // steal from other buckets

    release(&bcache[bucket].lock); // in order to avoid deadlock
    // steal lock is used to avoid deadlock
    acquire(&steallock);

    for (int i = 0; i < BCACHE_NBUCKET; i++) {
      if (i == bucket) {
        continue;
      }
      acquire(&bcache[i].lock);
      for (int j = 0; j < NBUF; j++) {
        b = &bcache[i].buf[j];
        if (b->refcnt == 0 && (lru == 0 || b->timestamp < lru->timestamp)) {
          lru = b;
        }
      }
      if (lru != 0) {
        release(&bcache[i].lock);
        break;
      }
      release(&bcache[i].lock);
    }

    if (lru == 0) {
      // release(&bcache[bucket].lock);
      release(&steallock);
      // release(&bcache[bucket].lock);
      panic("bget: no buffers");
    } else {
      lru->dev = dev;
      lru->blockno = blockno;
      lru->valid = 0;
      lru->refcnt = 1;
      lru->timestamp = ticks;
      acquiresleep(&lru->lock);
      release(&steallock);
      // release(&bcache[bucket].lock);
    }

  }

  // // release(&steallock);

  return lru;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  // get the bucket of the buffer
  uint bucket = hash(b->dev, b->blockno);

  acquire(&bcache[bucket].lock);
  b->refcnt--;
  b->timestamp = ticks;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    // not sure what to do here
  }
  
  release(&bcache[bucket].lock);
}

void
bpin(struct buf *b) {
  uint bucket = hash(b->dev, b->blockno);
  acquire(&bcache[bucket].lock);
  b->refcnt++;
  release(&bcache[bucket].lock);
}

void
bunpin(struct buf *b) {
  uint bucket = hash(b->dev, b->blockno);
  acquire(&bcache[bucket].lock);
  b->refcnt--;
  release(&bcache[bucket].lock);
}


