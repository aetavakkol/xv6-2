#include "spinlock.h"

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct stat;
struct superblock;
struct cache_info;
struct list_head;
struct filesystem;

// bio.c
void            binit(void);
struct buf*     bread(uint, uint);
void            brelse(struct buf*);
void            bwrite(struct buf*);

// console.c
void            consoleinit(void);
void            cprintf(char*, ...);
void            consoleintr(int(*)(void));
void            panic(char*) __attribute__((noreturn));

// exec.c
int             exec(char*, char**, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);

// fs.c
void            readsb(int dev, struct superblock *sb);
int             dirlink(struct inode*, char*, uint);
struct inode*   _dirlookup(struct inode*, char*, uint*);
struct inode*   dirlookup(struct inode*, char*, uint*);
struct inode*   ialloc(struct filesystem*, uint);
struct inode*   idup(struct inode*);
struct inode*   iget(struct filesystem*, uint);
struct filesystem* find_fs(uint);
void            iinit(void);
void            ilock(struct inode*);
void            iput(struct inode*);
void            iunlock(struct inode*);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
int             namecmp(const char*, const char*);
struct inode*   namei(char*);
struct inode*   nameiparent(char*, char*);
int             readi(struct inode*, char*, uint, uint);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, uint, uint);
int             type_to_mode(short);
int             get_current_permissions(struct inode*);
uint            bmap(struct inode *ip, uint bn);

// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// ioapic.c
void            ioapicenable(int irq, int cpu);
extern uchar    ioapicid;
void            ioapicinit(void);

// kalloc.c
char*           kalloc(void);
void            kfree(char*);
void            kinit1(void*, void*);
void            kinit2(void*, void*);
extern int      free_pages_count;

// kbd.c
void            kbdintr(void);

// lapic.c
int             cpunum(void);
extern volatile uint*    lapic;
void            lapiceoi(void);
void            lapicinit(void);
void            lapicstartap(uchar, uint);
void            microdelay(int);

// log.c
void            initlog(void);
void            log_write(struct buf*);
void            begin_trans();
void            commit_trans();

// mp.c
extern int      ismp;
int             mpbcpu(void);
void            mpinit(void);
void            mpstartthem(void);

// picirq.c
void            picenable(int);
void            picinit(void);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, char*, int);
int             pipewrite(struct pipe*, char*, int);

//PAGEBREAK: 16
// proc.c
struct proc*    copyproc(struct proc*);
void            exit(void);
void            exit_group(void);
void            kill_other_threads_in_group(void);
int             fork(void);
int             clone(void*, unsigned int);
int             growproc(int);
int             kill(int);
void            pinit(void);
void            procdump(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            sleep(void*, struct spinlock*);
void            userinit(void);
int             wait(void);
void            wakeup(void*);
void            yield(void);
struct mm_struct* get_empty_mm(void);
void            free_mm(struct mm_struct*);
struct proc*    get_proc_by_pid(int);
void*           mmap(void*, int, int, int, struct file*, int);
int             handle_pagefault(uint, uint);
void            free_mmaps(struct mm_struct* mm);

// swtch.S
void            swtch(struct context**, struct context*);

// spinlock.c
void            acquire(struct spinlock*);
void            getcallerpcs(void*, uint*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            pushcli(void);
void            popcli(void);

// string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             safestrlen(const char*, int);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// syscall.c
int             argint(int, int*);
int             argptr(int, char**, int);
int             argstr(int, char**);
int             fetchint(uint, int*);
int             fetchstr(uint, char**);
void            syscall(void);

// timer.c
void            timerinit(void);

// trap.c
void            idtinit(void);
extern uint     ticks;
void            tvinit(void);
extern struct spinlock tickslock;

// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartputc(int);

// vm.c
void            seginit(void);
void            kvmalloc(void);
void            vmenable(void);
pde_t*          setupkvm(void);
char*           uva2ka(pde_t*, char*);
int             allocuvm(pde_t*, uint, uint, uint);
int             deallocuvm(pde_t*, uint, uint);
void            freevm(pde_t*);
void            inituvm(pde_t*, char*, uint);
int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
pde_t*          copyuvm(pde_t*, uint);
void            switchuvm(struct proc*);
void            switchkvm(void);
int             copyout(pde_t*, uint, void*, uint);
void            clearpteu(pde_t *pgdir, char *uva);
int             set_pte_permissions(pde_t* pgdir, void* addr, uint perm);
int             get_pte_permissions(pde_t* pgdir, void* addr);
int             mappages(pde_t*, void*, uint, uint, int);
pte_t*          walkpgdir(pde_t *pgdir, const void *va, int alloc);

// kmalloc.c
void            init_caches(void);
struct cache_info* kmem_cache_create(unsigned int size);
void*           kmem_cache_alloc(struct cache_info*);
void            kmem_cache_free(void* mem);

// list.c

void            INIT_LIST_HEAD(struct list_head* list);
void            list_add(struct list_head* new, struct list_head* head);
void            list_add_tail(struct list_head* new, struct list_head* head);
void            list_del(struct list_head* entry);
void            list_del_init(struct list_head* entry);
int             list_empty(struct list_head* head);

// procfs.c
int mount_proc_fs(struct inode* ip, struct inode* parent);

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x)/sizeof((x)[0]))
