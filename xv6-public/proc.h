// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
};

extern struct cpu cpus[NCPU];
extern int ncpu;

//PAGEBREAK: 17
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is tha tthe caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and allocproc() manipulates it.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

// JUST added
typedef int pid_t;

// FOR reusing proc structure
typedef int thread_t;
typedef struct proc thread;
typedef enum procstate threadstate;

/*
struct thread_mutex_t{
  struct spinlock lock;
};
*/

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
struct proc {
  pde_t* pgdir;                // Page table
  
  uint sz;                     // Size of process memory (bytes)
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory

  // Debugging
  char name[16];               // Process name (debugging)

  //  MLFQ   SCHEDULING
  uint start_tick;
  int lev;
  int time_allotment;

  //  STRIDE SCHEDULING
  int share;
  int pass;

  // PROCESS
  int pid;    

  // LIGHT WEIGHT PROCESS
  int multi_threaded;
  int tid;
  int num_thread;
  int ret_val[NTHREAD];
  
  // Master thread would be index 0
  thread* threads[NTHREAD];
  int lock;
  // For master thread
  // struct spinlock lock; 

  // Debugging
  int first_scheduled;
  int from_trap;

  struct proc* master_thread;

};


// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

