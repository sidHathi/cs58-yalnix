# Project Directory Structure

- Yalnix Framework (copy from /thayerfs/courses/24winter/cosc058/workspace/yalnix_framework/)
- [Yalnix](./)
  - [Read Me](./README.md)
  - [Source Code](./src/)
    - [Syscalls Library](./src/syscall_handlers/)
      - [Process Coordination Function Headers](./src/syscall_handlers/process_coordination.h)
      - [Process Coordination](./src/syscall_handlers/process_coordination.c)
      - [I/O Function Headers](./src/syscall_handlers/input_output.h)
      - [I/O](./src/syscall_handlers/input_output.c)
      - [Interprocess Communication Function Headers](./src/syscall_handlers/ipc.h)
      - [Interprocess Communication](./src/syscall_handlers/ipc.c)
      - [Synchronization Function Headers](./src/syscall_handlers/synchronization.h)
      - [Synchronization](./src/syscall_handlers/synchronization.c)
    - [Trap Kernel Handler Function Headers](./src/trap_kernel_handler.h)
    - [Trap Kernel Handler](./src/trap_kernel_handler.c)
    - [Kernel Brk and Start Function Headers](./src/kernel.h)
    - [Kernel Brk and Start](./src/kernel.c)

  - [Testing](./tests/)
    - TBD

# To-Do For Checkpoint 1

## Syscall Handler Library Header Comments and Function Pseudocode

Note that these function names all start with "kernel" to differentiate them from their userland wrapper counterparts. This is recommended on page 43 of the manual.

- Add comments to:
  - [Process Coordination Function Headers](./src/syscall_handlers/process_coordination.h)
  - [I/O Function Headers](./src/syscall_handlers/input_output.h)
  - [Interprocess Communication Function Headers](./src/syscall_handlers/ipc.h)
  - [Synchronization Function Headers](./src/syscall_handlers/synchronization.h)
- Add pseudocode to:
  - [Process Coordination](./src/syscall_handlers/process_coordination.c)
  - [I/O](./src/syscall_handlers/input_output.c)
  - [Interprocess Communication](./src/syscall_handlers/ipc.c)
  - [Synchronization](./src/syscall_handlers/synchronization.c)

## Trap Handler Needs Header Comment and Function Pseudocode

- Add header comment to [Trap Handler Function Header](./src/trap_handler.h)
- Add pseudocode to [Trap Handler](./src/trap_handler.c)
