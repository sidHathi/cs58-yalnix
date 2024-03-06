# Yalnix Operating System

Contributors: Brody Thompson, Sid Hathi, and Asher Vogel

## Design

### The Hardware

The Yalnix operating system is built on top of simulated hardware written by Dave Johnson (Rice University), Adam Salem (Dartmouth College), and Sean Smith (Dartmouth College). This infrastructure, written in C, exposes a number of hardware services including general purpose and priveleged registers, physical memory, an MMU with a built-in TLB, a clock , and terminals. The "hardware" is designed to run on a Unix flavor system and provides the services and resources to the Yalnix operating system.

### Data Structures: [Directory](./src/datastructures/)

We built strongly cohesive data structures to minimize redundancies and enable comprehensive unit testing.

#### Set: [Source Code](./src/datastructures/set.c) and [Header File](./src/datastructures/set.h)

The set structure stores key-value pairs in a linked list structure. Keys are integers, and values are ```void*``` pointers. This data structure supports insertion, deletion, and querying in linear time. See the header file for more details on usage.

#### Queue: [Source Code](./src/datastructures/queue.c) and [Header File](./src/datastructures/queue.h)

The queue structure is a classic FIFO queue implemented with a linked list. The data in each node is stored as a ```void*``` pointer. This data structure supports traditional queue push and queue pop operations. See the header file for more details on usage.

#### TTY State: [Source Code](./src/datastructures/tty_state.c) and [Header File](./src/datastructures/tty_state.h)

The TTY state structure stores information on the state of a specific terminal in the Yalnix system. There is a TTY state data structure allocated for each terminal the system exposes.

#### Lock: [Header File](./src/datastructures/lock.h)

The lock structure is comparable to the pthreads mutex structure. It is a process coordination tool used to ensure only one process has access to a resource at any given time.

#### Condition Variable: [Header File](./src/datastructures/cvar.h)

The cvar structure is comparable to the pthreads cvar structure. It is designed to be used in conjunction with locks to facilitate efficient multithreading.

#### Pipe: [Header File](./src/datastructures/pipe.h)

The pipe structure fascilitates inter-process communication via an allocated ```char*``` buffer. The data structure keeps a queue of processes waiting to read from the pipe to minimize the risk of race conditions.

#### IPC Wrapper: [Source Code](./src/datastructures/ipc_wrapper.c) and [Header File](./src/datastructures/ipc_wrapper.h)

The IPC wrapper structure, short for inter-process communication wrapper, is designed to be allocated as a global by the kernel itself. It contains a set of locks, a set of cvars, and a set of pipes, as well as utility functions for creating and removing locks, cvars, and pipes. The data structure contains an integer that corresponds to the ID of the next structure allocated. This ensure that no two inter-process communication structure (locks, cvars, pipes) share an ID.

#### Process Control Block (PCB): [Source Code](./src/datastructures/pcb.c) and [Header File](./src/datastructures/pcb.h)

The PCB data structure stores all information needed by the kernel to manipulate processes. This includes the user context (stack pointer, program counter, etc.), process ID, kernel context, etc. See the header file for more information on what is in the PCB.

### The Kernel: [Source Code](./src/kernel.c) and [Header File](./src/kernel.h)

The ```KernelStart``` function is responsible for the pivotal functionality of the Yalnix kernel's boot sequence. ```KernelStart``` handles the following:
  - Set up the region 0 page table with 2 frames for the kernel stack
  - Allocate memory for the free frame queue and enqueue all frames not used by the region 0 page table
  - Enable virtual memory
  - Allocate memory for the global process ready queue, a set for all blocked processes, a for all delayed processes, and a set for all dead processes
  - Register all trap handlers
  - Set up idle PCB for when no processes are ready to run
  - Load init program into current PCB

The Yalnix kernel prioritizes fairness and efficiency. The system implements round-robin scheduling, allocating CPU time slices evenly across all active processes. Each process is granted a slice of execution time, with the scheduler swiftly transitioning between tasks to maintain responsiveness and optimize throughput. Round robin scheduling is handled by ```ScheduleNextProcess```. Note that this function only moves the outgoing process's PCB to the appropriate data structure (ready queue, blocked processes, etc.) and performs the kernel context swtich. The calling process is responsible for copying the user context.

### Traps: [Source Code](./src/traps.c) and [Header File](./src/traps.h)

There is a trap for each type of syscall exposed to the user, as well as for illegal behavior such as illegal memory touching or a math error. Each traps invokes the appropriate syscall handler if necessary and then copies the user context before returning to the user. In the case where a process does something dangerous (e.g. attempt to write to kernel heap or dereference null pointer), the trap will abort that process. See the header file for more details on each trap.

### [Syscalls](./src/syscall_handlers/)

The syscall handlers are invoked when a user process traps down to the kernel. The header files for these syscall handlers contain details on how they behave.

### [Testing](./tests/)

We ran all tests in this testing directory. To try a test called "my_test," run ```./yalnix -x tests/my_test```. Our tests look at the following scenarios:

- Fork + Exec
- Fork + Wait
- Fork bombing
- Massive user heap
- Pipe buffer overflow
- Null pointer dereferencing 

### [User Programs](./src/programs/)

  - #### [Idle](./src/programs/idle.h)

### [Syscall Handlers](./src/syscall_handlers/)

  - #### [Process Coordination](./src/syscall_handlers/process_coordination.h)

  - #### [Input/Output](./src/syscall_handlers/input_output.h)

  - #### [Inter Process Communication](./src/syscall_handlers/ipc.h)

  - #### [Synchronization](./src/syscall_handlers/synchronization.h)

## Usage

Usage of the Yalnix operating system requires access to ```yalnix_framework```, the hardware simulation.

```
git clone git@github.com:sidHathi/cs58-yalnix.git
ln -s /path/to/yalnix_framework .
cd cs58-yalnix
make
./yalnix executable_program
```

To run your own user program, simply add the name of the C file to [line 22](./src/Makefile) at the end of ```U_SRCS``` before compiling.

## Acknowledgements

Huge thanks to Sean Smith and Alex Chen for guiding us along this journey.