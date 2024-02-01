# Project Directory Structure

- Yalnix Framework (copy from /thayerfs/courses/24winter/cosc058/workspace/yalnix_framework/)
- [Yalnix](./)
  - [Read Me](./README.md)
  - [Source Code](./src/)
    - [Syscall Handlers](./src/syscall_handlers/)
      - [Process Coordination Function Headers](./src/syscall_handlers/process_coordination.h)
      - [Process Coordination](./src/syscall_handlers/process_coordination.c)
      - [I/O Function Headers](./src/syscall_handlers/input_output.h)
      - [I/O](./src/syscall_handlers/input_output.c)
      - [Interprocess Communication Function Headers](./src/syscall_handlers/ipc.h)
      - [Interprocess Communication](./src/syscall_handlers/ipc.c)
      - [Synchronization Function Headers](./src/syscall_handlers/synchronization.h)
      - [Synchronization](./src/syscall_handlers/synchronization.c)
    - [Utils](./src/util/)
      - [Queue Struct and Function Headers](./src/util/queue.h)
      - [Queue Struct and Functions](./src/util/queue.c)
    - [Trap Handlers](./src/traps.c)
    - [Kernel Brk and Start Function Headers](./src/kernel.h)
    - [Kernel Brk and Start](./src/kernel.c)
  - [Testing](./tests/)
    - TBD

# Boot Process

The pseudo code for booting up the Yalnix kernel is outlined in the KernelStart function in [kernel.c](./src/kernel.c). At the time of boot the only memory being used is in the kernel data and the kernel heap, as no user processes are yet running.

## Kernel Data

The kernel data portion of memory will contain static data that doesn't grow. This includes:

- Region 0 page table
- Interrupt vector table
- Initial region 1 page table

## Kernel Heap

Since the kernel heap is dynamic and isn't erased during context switiching, this is the natural place to store the majority of critical system information:

- Free frame queue
- Ready queue (queue of PCB's waiting to run)
- Blocked array (array of PCB's currently blocked)
- Dead array (array of PCB's that have been terminated)
- Current process (PCB of current running process)
- Region 1 page table