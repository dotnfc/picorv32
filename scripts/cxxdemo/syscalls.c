// Based on riscv newlib libgloss/riscv/machine/syscall.h
// Changes by Clifford Wolf

//========================================================================
// syscalls.c : Newlib operating system interface                       
//========================================================================
// This is the maven implementation of the narrow newlib operating
// system interface. It is based on the minimum stubs in the newlib
// documentation, the error stubs in libnosys, and the previous scale
// implementation. Please do not include any additional system calls or
// other functions in this file. Additional header and source files
// should be in the machine subdirectory.
//
// Here is a list of the functions which make up the operating system
// interface. The file management instructions execute syscall assembly
// instructions so that a proxy kernel (or the simulator) can marshal up
// the request to the host machine. The process management functions are
// mainly just stubs since for now maven only supports a single process.
//
//  - File management functions
//     + open   : (v) open file
//     + lseek  : (v) set position in file
//     + read   : (v) read from file
//     + write  : (v) write to file
//     + fstat  : (z) status of an open file
//     + stat   : (z) status of a file by name
//     + close  : (z) close a file
//     + link   : (z) rename a file
//     + unlink : (z) remote file's directory entry
//
//  - Process management functions
//     + execve : (z) transfer control to new proc
//     + fork   : (z) create a new process 
//     + getpid : (v) get process id 
//     + kill   : (z) send signal to child process
//     + wait   : (z) wait for a child process
//     
//  - Misc functions
//     + isatty : (v) query whether output stream is a terminal
//     + times  : (z) timing information for current process
//     + sbrk   : (v) increase program data space
//     + _exit  : (-) exit program without cleaning up files
//
// There are two types of system calls. Those which return a value when
// everything is okay (marked with (v) in above list) and those which
// return a zero when everything is okay (marked with (z) in above
// list). On an error (ie. when the error flag is 1) the return value is
// always an errno which should correspond to the numbers in
// newlib/libc/include/sys/errno.h 
//
// Note that really I think we are supposed to define versions of these
// functions with an underscore prefix (eg. _open). This is what some of
// the newlib documentation says, and all the newlib code calls the
// underscore version. This is because technically I don't think we are
// supposed to pollute the namespace with these function names. If you
// define MISSING_SYSCALL_NAMES in xcc/src/newlib/configure.host
// then xcc/src/newlib/libc/include/_syslist.h will essentially define
// all of the underscore versions to be equal to the non-underscore
// versions. I tried not defining MISSING_SYSCALL_NAMES, and newlib
// compiled fine but libstdc++ complained about not being able to fine
// write, read, etc. So for now we do not use underscores (and we do
// define MISSING_SYSCALL_NAMES).
//
// See the newlib documentation for more information 
// http://sourceware.org/newlib/libc.html#Syscalls

#include <sys/stat.h>
#include <sys/times.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>

//------------------------------------------------------------------------
// environment                                                          
//------------------------------------------------------------------------
// A pointer to a list of environment variables and their values. For a
// minimal environment, this empty list is adequate. We used to define
// environ here but it is already defined in
// xcc/src/newlib/libc/stdlib/environ.c so to avoid multiple definition
// errors we have commented this out for now.
//
// char* __env[1] = { 0 };
// char** environ = __env;
              
//------------------------------------------------------------------------
// open                                                                 
//------------------------------------------------------------------------
// Open a file.

int open(const char* name, int flags, int mode)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// openat                                                                
//------------------------------------------------------------------------
// Open file relative to given directory
int openat(int dirfd, const char* name, int flags, int mode)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// lseek                                                                
//------------------------------------------------------------------------
// Set position in a file.

off_t lseek(int file, off_t ptr, int dir)
{
	errno = ESPIPE;
	return -1;
}

//----------------------------------------------------------------------
// read                                                                 
//----------------------------------------------------------------------
// Read from a file.

ssize_t read(int file, void *ptr, size_t len)
{
	// always EOF
	return 0;
}

//------------------------------------------------------------------------
// write                                                                
//------------------------------------------------------------------------
// Write to a file.

ssize_t write(int file, const void *ptr, size_t len)
{
	unsigned int *p = (unsigned int*)0x10000000;
	const void *eptr = ptr + len;
	while (ptr != eptr)
		*p = *(char*)(ptr++);
	return len;
}

//------------------------------------------------------------------------
// fstat                                                                
//------------------------------------------------------------------------
// Status of an open file. The sys/stat.h header file required is
// distributed in the include subdirectory for this C library.

int fstat(int file, struct stat *st)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// stat                                                                 
//------------------------------------------------------------------------
// Status of a file (by name).

int stat(const char *file, struct stat *st)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// lstat                                                                 
//------------------------------------------------------------------------
// Status of a link (by name).

int lstat(const char *file, struct stat *st)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// fstatat                                                                 
//------------------------------------------------------------------------
// Status of a file (by name) in a given directory.

int fstatat(int dirfd, const char *file, struct stat *st, int flags)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// access                                                                 
//------------------------------------------------------------------------
// Permissions of a file (by name).

int access(const char *file, int mode)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// faccessat                                                                 
//------------------------------------------------------------------------
// Permissions of a file (by name) in a given directory.

int faccessat(int dirfd, const char *file, int mode, int flags)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// close                                                                
//------------------------------------------------------------------------
// Close a file.

int close(int file) 
{
	// close is always ok
	return 0;
}

//------------------------------------------------------------------------
// link                                                                 
//------------------------------------------------------------------------
// Establish a new name for an existing file.

int link(const char *old_name, const char *new_name)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// unlink                                                               
//------------------------------------------------------------------------
// Remove a file's directory entry.

int unlink(const char *name)
{
	errno = ENOENT;
	return -1;
}

//------------------------------------------------------------------------
// execve                                                               
//------------------------------------------------------------------------
// Transfer control to a new process. Minimal implementation for a
// system without processes from newlib documentation.

int execve(const char *name, char *const argv[], char *const env[])
{
	errno = ENOMEM;
	return -1;
}

//------------------------------------------------------------------------
// fork                                                                 
//------------------------------------------------------------------------
// Create a new process. Minimal implementation for a system without
// processes from newlib documentation.

int fork() 
{
	errno = EAGAIN;
	return -1;
}

//------------------------------------------------------------------------
// getpid                                                               
//------------------------------------------------------------------------
// Get process id. This is sometimes used to generate strings unlikely
// to conflict with other processes. Minimal implementation for a
// system without processes just returns 1.

int getpid() 
{
	return 1;
}

//------------------------------------------------------------------------
// kill                                                                 
//------------------------------------------------------------------------
// Send a signal. Minimal implementation for a system without processes
// just causes an error.

int kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

//------------------------------------------------------------------------
// wait                                                                 
//------------------------------------------------------------------------
// Wait for a child process. Minimal implementation for a system without
// processes just causes an error.

int wait(int *status)
{
	errno = ECHILD;
	return -1;
}

//------------------------------------------------------------------------
// isatty                                                               
//------------------------------------------------------------------------
// Query whether output stream is a terminal. For consistency with the
// other minimal implementations, which only support output to stdout,
// this minimal implementation is suggested by the newlib docs.

int isatty(int file)
{
	return 1;
}

//------------------------------------------------------------------------
// times                                                                
//------------------------------------------------------------------------
// Timing information for current process. From
// newlib/libc/include/sys/times.h the tms struct fields are as follows:
//
//  - clock_t tms_utime  : user clock ticks
//  - clock_t tms_stime  : system clock ticks
//  - clock_t tms_cutime : children's user clock ticks
//  - clock_t tms_cstime : children's system clock ticks
//
// Since maven does not currently support processes we set both of the
// children's times to zero. Eventually we might want to separately
// account for user vs system time, but for now we just return the total
// number of cycles since starting the program.

clock_t times(struct tms *buf)
{
	// when called for the first time, initialize t0
	static struct timeval t0;
	if (t0.tv_sec == 0)
		gettimeofday(&t0, 0);

	struct timeval t;
	gettimeofday(&t, 0);

	long long utime = (t.tv_sec - t0.tv_sec) * 1000000 + (t.tv_usec - t0.tv_usec);
	buf->tms_utime = utime * CLOCKS_PER_SEC / 1000000;
	buf->tms_stime = buf->tms_cstime = buf->tms_cutime = 0;

	return -1;
}

//----------------------------------------------------------------------
// gettimeofday                                                                 
//----------------------------------------------------------------------
// Get the current time.  Only relatively correct.

int gettimeofday(struct timeval *tp, void *tzp)
{
	asm volatile ("sbreak");
	__builtin_unreachable();
}

//----------------------------------------------------------------------
// ftime                                                                 
//----------------------------------------------------------------------
// Get the current time.  Only relatively correct.

int ftime(struct timeb *tp)
{
	tp->time = tp->millitm = 0;
	return 0;
}

//----------------------------------------------------------------------
// utime
//----------------------------------------------------------------------
// Stub.

int utime(const char *path, const struct utimbuf *times)
{
	return -1;
}

//----------------------------------------------------------------------
// chown
//----------------------------------------------------------------------
// Stub.

int chown(const char *path, uid_t owner, gid_t group)
{
	return -1;
}

//----------------------------------------------------------------------
// chmod
//----------------------------------------------------------------------
// Stub.

int chmod(const char *path, mode_t mode)
{
	return -1;
}

//----------------------------------------------------------------------
// chdir
//----------------------------------------------------------------------
// Stub.

int chdir(const char *path)
{
	return -1;
}

//----------------------------------------------------------------------
// getcwd
//----------------------------------------------------------------------
// Stub.

char *getcwd(char *buf, size_t size)
{
	return NULL;
}

//----------------------------------------------------------------------
// sysconf
//----------------------------------------------------------------------
// Get configurable system variables

long sysconf(int name)
{
	switch (name) {
	case _SC_CLK_TCK:
		return CLOCKS_PER_SEC;
	}

	return -1;
}

//----------------------------------------------------------------------
// sbrk                                                                 
//----------------------------------------------------------------------
// Increase program data space. As malloc and related functions depend
// on this, it is useful to have a working implementation. The following
// is suggested by the newlib docs and suffices for a standalone
// system.

void *sbrk(ptrdiff_t incr)
{
	extern unsigned char _end[];	// Defined by linker
	static unsigned long heap_end;

	if (heap_end == 0)
		heap_end = (long)_end;

	// if (syscall_errno(SYS_brk, heap_end + incr, 0, 0, 0) != heap_end + incr)
	//	return (void *)-1;

	heap_end += incr;
	return (void *)(heap_end - incr);
}

//------------------------------------------------------------------------
// _exit                                                                
//------------------------------------------------------------------------
// Exit a program without cleaning up files.

void _exit(int exit_status)
{
	asm volatile ("sbreak");
	__builtin_unreachable();
}
