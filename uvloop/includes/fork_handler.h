
#ifdef _WIN32
/* No fork() in windows - so ignore this */
#define PyOS_BeforeFork() 0
#define PyOS_AfterFork_Parent() 0
#define PyOS_AfterFork_Child() 0
#define pthread_atfork(prepare, parent, child) 0
#include <winsock2.h>
#else
#include <pthread.h>
#include <sys/socket.h>

#endif

typedef void (*OnForkHandler)();

OnForkHandler __forkHandler = NULL;

/* Auxiliary function to call global fork handler if defined.

Note: Fork handler needs to be in C (not cython) otherwise it would require
GIL to be present, but some forks can exec non-python processes.
*/
void handleAtFork(void) {
    if (__forkHandler != NULL) {
        __forkHandler();
    }
}


void setForkHandler(OnForkHandler handler)
{
    __forkHandler = handler;
}


void resetForkHandler(void)
{
    __forkHandler = NULL;
}
