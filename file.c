//
// File descriptors
//

#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "file.h"
#include "pipe.h"
#include "stat.h"
#include "errno.h"

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
// Also increments readopen or writeopen, if file is an
// end of a pipe.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  if (f->type == FD_FIFO) {
    acquire(&f->pipe->lock);
    f->pipe->writeopen += f->writable;
    f->pipe->readopen += f->readable;
    release(&f->pipe->lock);
  }
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  int should_delete = 0;
  if(f->type == FD_FIFO){
    struct pipe* p = f->pipe;
    // If we are the last process using this end of the pipe,
    // wake up other processes on the other end, so that they
    // could act accordingly.
    acquire(&p->lock);
    if(f->writable && --p->writeopen <= 0){
      p->writeopen = 0;
      wakeup(&p->nread);
    } else if(!f->writable && --p->readopen <= 0){
      p->readopen = 0;
      wakeup(&p->nwrite);
    }
    if (!p->writeopen && !p->readopen) {
      should_delete = 1;
    }
    release(&p->lock);
  }
  if(--f->ref > 0 && !should_delete){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);
  
  if(ff.type == FD_FIFO){
    // Close our end of the pipe,
    // and set read_file and write_file of our inode to 0,
    // so that on the next open of the FIFO we will create the pipe again.
    pipeclose(ff.pipe, ff.writable);
    ilock(ff.ip);
    ff.ip->read_file->ref = 0;
    ff.ip->write_file->ref = 0;
    ff.ip->read_file = 0;
    ff.ip->write_file = 0;
    iunlock(ff.ip);
  } else if(ff.type == FD_PIPE){
    pipeclose(ff.pipe, ff.writable);
  } else if(ff.type == FD_INODE){
    begin_trans();
    iput(ff.ip);
    commit_trans();
  }
}

// Get metadata about file f.
int
filestat(struct file *f, struct stat *st)
{
  ilock(f->ip);
  stati(f->ip, st);
  iunlock(f->ip);
  return 0;
}

// Read from file f.
int
fileread(struct file *f, char *addr, int n)
{
  int r;

  if(f->readable == 0)
    return -EBADF;
  if(f->type == FD_PIPE || f->type == FD_FIFO)
    return piperead(f->pipe, addr, n);
  if(f->type == FD_INODE){
    ilock(f->ip);
    if((r = readi(f->ip, addr, f->off, n)) > 0)
      f->off += r;
    iunlock(f->ip);
    return r;
  }
  panic("fileread");
}

//PAGEBREAK!
// Write to file f.
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -EBADF;
  if(f->type == FD_PIPE || f->type == FD_FIFO)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // write a few blocks at a time to avoid exceeding
    // the maximum log transaction size, including
    // i-node, indirect block, allocation blocks,
    // and 2 blocks of slop for non-aligned writes.
    // this really belongs lower down, since writei()
    // might be writing a device like the console.
    int max = ((LOGSIZE-1-1-2) / 2) * 512;
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_trans();
      ilock(f->ip);
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      commit_trans();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -EIO;
  }
  panic("filewrite");
}
