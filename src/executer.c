#define _GNU_SOURCE

#include "executer.h"
#include "stack.h"
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>

void execute(uint64_t address) {
    void* sp = get_sp();
    int flags = CLONE_VM | CLONE_SIGHAND | CLONE_THREAD | 
                CLONE_FS | CLONE_FILES | CLONE_SETTLS;
    pid_t pid = clone(((int(*)(void*))address), sp, flags, NULL);
    
    waitpid(pid, NULL, __WALL);
}