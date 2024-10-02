# Notes

> I have finished this lab in October, 2023 but I did not keep a copy of the code on github. Therefore this is a revisit and all the code is hosted and showcased in this repo.

## User level threading system

This lab enables a user-level threading system in xv6. We wish to provide such an interface to the user as

```
thread_create(func1);
thread_create(func2);
...
thread_schedule();
```

where `thread_create()` gets a thread from the thread pool and ***sets up*** the contexts (mainly return address, stack pointers) to prepare for the initiation of the threads. 

To run the threads, `thread_schedule()` finds a runnable thread and calls `thread_switch()` so that the CPU starts running the found thread. A problem here is that `thread_schedule()` only wakes threads one at a time and does not account for any priority. This lab thus mandates that the threading functions should call `thread_schedule()` upon finishing. Also, concurrency is not achieved here, since the OS does not force threads to give up CPU time. Rather, this is simulated by having every threading function calling `thread_yield()`.

## Difference between contexts and trapframes

Both `struct context` and `struct trapframe` are defined in `kernel/proc.h` and both structs are used to save a set of registers. They both serve to save the state of a process. Notably, `context` preserves much fewer registers than `trapframe` in the code.

- Contexts are used for kernel context switches, saving the state of the CPU registers when switching between different processes and threads. It includes the values of the callee-saved registers and `ra` & `sp`. On a higher level, contexts come into use when a process yields the CPU, or when the scheduler decides to switch to a different process.

- Trapframes are used when an interrupt/exception/system call comes in. A trapframe should capture the state of the CPU at the moment of the trap, so that the kernel can handle the trap event and resume the interrupted event later correctly. Therefore, a trapframe includes **all** the CPU registers (incl. `pc`, `sp`...).

> Why do syscalls use trapframes rather than context switches? Syscalls involve a transition from user mode to kernel mode. Theoretically, it might not be necessary to save *all* the CPU registers. It's more of a safe move to do so.