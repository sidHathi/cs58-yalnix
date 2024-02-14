#include <yalnix.h>
#include <ykernel.h>
#include "../datastructures/linked_list.h"

/*
* Fork:
*
* Fork is how new processes are created in Yalnix. The memory image of the new process (the child) is a copy
* of that of the process calling Fork (the parent). When the Fork call completes, both the parent process and
* the child process return (separately) from the syscall as if they had been the one to call Fork, since the child
* is a copy of the parent. The only distinction is the fact that the return value in the calling (parent) process is
* the process ID of the new (child) process, while the value returned in the child is 0. If, for any reason, the new
* process cannot be created, this syscall instead returns the value ERROR to the calling process.
*/
int ForkHandler(void);


/*
* Exec:
*
* Replace the currently running program in the calling process’s memory with the program stored in the file
* named by filename. The argument argvec points to a vector of arguments to pass to the new program as
* its argument list. The new program is called as
*
*     int main(argc, argv)
*     int argc;
*     char *argv[];
*
* where argc is the argument count and argv is an array of character pointers to the arguments themselves. The
* strings pointed to by the entries in argv are copied from the strings pointed to by the argvec array passed to
* Exec, and argc is a count of entries in this array before the first NULL entry, which terminates the argument
* list. When the new program begins running, its argv[argc] is NULL. By convention the first argument in the
* argument list passed to a new program (argvec[0]) is also the name of the new program to be run, but this is
* just a convention; the actual file name to run is determined only by the filename argument. On success, there
* is no return from this call in the calling program, and instead, the new program begins executing in this process
* at its entry point, and its main(argc, argv) routine is called as indicated above.
* On failure, if the calling process has not been destroyed already, this call returns ERROR and does not run the
* new program. 
*/
int ExecHandler(char* filename, char** argvec);


/*
* Exit: 
*
* Exit is the normal means of terminating a process. The current process is terminated, the integer status
* value is saved for possible later collection by the parent process on a call to Wait. All resources used by the
* calling process will be freed, except for the saved status information. This call can never return.
* When a process exits or is aborted, if it has children, they should continue to run normally, but they will no
* longer have a parent.
*/
void ExitHandler(int status);


/*
* Wait:
*
* Collect the process ID and exit status returned by a child process of the calling program.
* If the caller has an exited child whose information has not yet been collected via Wait, then this call will return
* immediately with that information.
* If the calling process has no remaining child processes (exited or running), then this call returns immediately,
* with ERROR.
* Otherwise, the calling process blocks until its next child calls exits or is aborted; then, the call returns with the
* exit information of that child.
* On success, the process ID of the child process is returned. If status ptr is not null, the exit status of the
* child is copied to that address.
*/
int WaitHandler(int *status_ptr);


/*
* GetPid:
*
* Returns the process ID of the calling process.
*/
int GetPidHandler(void);


/*
* Brk:
*
* Brk sets the operating system’s idea of the lowest location not used by the program (called the “break”) to addr
* (rounded up to the next multiple of PAGESIZE bytes). This call has the effect of allocating or deallocating
* enough memory to cover only up to the specified address. Locations not less than addr and below the stack
* pointer are not in the address space of the process and will thus cause an exception if accessed.
* The value 0 is returned on success. If any error is encountered (for example, if not enough memory is available
* or if the address addr is invalid), the value ERROR is returned.
*/
int BrkHandler(void* addr);

/*
* Delay:
*
* The calling process is blocked until at least clock ticks clock interrupts have occurred after the call. Upon
* completion of the delay, the value 0 is returned.
* If clock ticks is 0, return is immediate. If clock ticks is less than 0, time travel is not carried out, and
* ERROR is returned instead.

*/
int KernelDelay(int clock_ticks);