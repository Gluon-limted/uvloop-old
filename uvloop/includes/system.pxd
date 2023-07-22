from libc.stdint cimport int8_t, uint64_t

IF UNAME_SYSNAME == "Windows":
    include "windows.pxi"
ELSE:
    include "posix.pxi"


cdef extern from "includes/compat.h" nogil:

    cdef int EWOULDBLOCK

    cdef int PLATFORM_IS_APPLE
    cdef int PLATFORM_IS_LINUX
    cdef int PLATFORM_IS_WINDOWS

    struct epoll_event:
        # We don't use the fields
        pass

    int EPOLL_CTL_DEL
    int epoll_ctl(int epfd, int op, int fd, epoll_event *event)
    object MakeUnixSockPyAddr(sockaddr_un *addr)
    void DebugBreak()
    void DbgBreak()
    void stdio_container_init(void *pipe, int fd)
    void process_init(void *process)
    void CloseIOCP(void* loop)
    void PrintAllHandle(void* loop)
    int create_tcp_socket()


cdef extern from "includes/fork_handler.h":

    uint64_t MAIN_THREAD_ID
    int8_t MAIN_THREAD_ID_SET
    ctypedef void (*OnForkHandler)()
    void handleAtFork()
    void setForkHandler(OnForkHandler handler)
    void resetForkHandler()
    void setMainThreadID(uint64_t id)
