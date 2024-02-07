# Yalnix Operating System

By: Brody Thompson, Sid Hathi, and Asher Vogel

## Repo Structure

### [Makefile](./Makefile)

This wrapper Makefile defines two targets. Run ```make``` to build the ```yalnix``` executable in the root directory. Run ```make clean``` to remove all executables, object files, and library binaries.

### [src](./src)

#### - [Data Structures](./src/datastructures/)

This directory contains header and C files that define the key data structures for Yalnix: locks, cvars, pipes, queues, pcb, and memory cache.

#### - [Programs](./src/programs/)

This directory contains userland programs that can run on Yalnix. For now, this just contains the Idle program.

#### - [Syscall Handlers](./src/syscall_handlers/)

This directory contains the necessary syscall handlers that are invoked when a trap kernel is triggered.

### [Kernel C Code](./src/kernel.c) and [Kernel Headers](./src/kernel.h)

These files contain the driver code for booting up the kernel, scheduling processes, and allocating memory in the page tables.

### [Traps C Code](./src/traps.c) and [Traps Headers](./src/traps.h)

These files contain the trap handlers that form the interrupt vector table. They also define a function ```RegisterTrapHandlers``` that can be called by ```KernelStart``` to write the interrupt vector table pointer to the appropriate priveledged register.