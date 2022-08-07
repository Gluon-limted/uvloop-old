#include <errno.h>
#include <stddef.h>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include "Python.h"
#include "uv.h"


#ifndef EWOULDBLOCK
#define EWOULDBLOCK EAGAIN
#endif

#ifdef __APPLE__
#define PLATFORM_IS_APPLE 1
#else
#define PLATFORM_IS_APPLE 0
#endif


#ifdef __linux__
#  define PLATFORM_IS_LINUX 1
#  include <sys/epoll.h>
#else
#  define PLATFORM_IS_LINUX 0
#  define EPOLL_CTL_DEL 2
struct epoll_event {
    uint32_t events;
    void *ptr;
};
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event) {
    return 0;
};
#endif

#ifdef _WIN32
#pragma warning( disable : 4311 4312)

#include <stdio.h>

#include <windows.h>

#define PLATFORM_IS_WINDOWS 1

struct sockaddr_un{
    unsigned short sun_family;
    char           sun_path[108];
};

int GetFileHandle(int fd) {
    if (fd < 0) {
        return (int)INVALID_HANDLE_VALUE;
    }
    return (int)_get_osfhandle(fd);
}

int GetStdIn(void){
    return (int)GetStdHandle(STD_INPUT_HANDLE);
}

int GetStdOut(void){
    return (int)GetStdHandle(STD_OUTPUT_HANDLE);
}

int GetStdErr(void){
    return (int)GetStdHandle(STD_ERROR_HANDLE);
}

int create_pipe_ex(long* pipeRd, long* pipeWr){
    SECURITY_ATTRIBUTES saAttr = {0};
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    int err = CreatePipe((HANDLE*)pipeRd, (HANDLE*)pipeWr, &saAttr, 0);
    return err;
}

long file_redirect_stdio(HANDLE fd){
    char* mode = "w";
    int hcon = _open_osfhandle((intptr_t)fd, _O_TEXT);
    FILE *fp = _fdopen(hcon, mode);
    setvbuf(fp, NULL, _IONBF, 0);
    SetHandleInformation((HANDLE)_fileno(fp), HANDLE_FLAG_INHERIT, FALSE);
    return (long)_fileno(fp);
}


int setsockopt_reuseport(int fd) {
    int reuseport_flag = 1;
    int err = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&reuseport_flag, sizeof(reuseport_flag));
    return (err == SOCKET_ERROR) ? err : setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseport_flag, sizeof(reuseport_flag));
}

int is_sigchild(int sig) {
    return FALSE;
}

int socketpair(int domain, int type, int protocol, int fds[2]) {
    uv_os_sock_t socket_vector[2];
    int ret = uv_socketpair(type, protocol, socket_vector, UV_NONBLOCK_PIPE, UV_NONBLOCK_PIPE);
    fds[0] = (int)socket_vector[0];
    fds[1] = (int)socket_vector[1];
    return ret;
}

int socketpair_ex(int domain, int type, int protocol, int fds[2]) {
    SOCKET listener = INVALID_SOCKET;
    SOCKADDR_IN name = { 0 };
    int namelen = sizeof(name);
    int reuse = 1;
    int err;
    DWORD flags = 0;

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
        return WSAGetLastError();

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    name.sin_port = 0;

    fds[0] = fds[1] = INVALID_SOCKET;
    do {
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse, sizeof(reuse)) != 0)
            break;
        if (bind(listener, (SOCKADDR*)&name, sizeof(name)) != 0)
            break;
        if (getsockname(listener, (SOCKADDR*)&name, &namelen) != 0)
            break;
        if (listen(listener, 1) != 0)
            break;
        fds[0] = (int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (fds[0] == INVALID_SOCKET)
            break;
        if (connect(fds[0], (SOCKADDR*)&name, sizeof(name)) != 0)
            break;
        fds[1] = (int)accept(listener, NULL, NULL);
        if (fds[1] == INVALID_SOCKET)
            break;

        closesocket(listener);
        return 0;
    } while (0);

    err = WSAGetLastError();
    if (listener != INVALID_SOCKET)
        closesocket(listener);
    if (fds[0] != INVALID_SOCKET)
        closesocket(fds[0]);
    if (fds[1] != INVALID_SOCKET)
        closesocket(fds[1]);

    return err;
}


void close_ex(HANDLE fd) {
//GetFileType(handle) == FILE_TYPE_CHAR)
    int err = 0;
    if (GetFileType((HANDLE)fd) == FILE_TYPE_CHAR) {
        printf("close(%p) (%d)\n", fd, GetFileType(fd));
        err = CloseHandle(fd);
    } else if (GetFileType((HANDLE)fd) == FILE_TYPE_PIPE) {
        printf("closesocket(%p) (%d)\n", fd, GetFileType(fd));
        err = closesocket((SOCKET)fd);
    } else {
        printf("CloseHandle(%p) (%d)\n", fd, GetFileType((HANDLE)fd));
        err = CloseHandle((HANDLE)fd);
    }

    if (err) {
        printf("error(%d) %d/%u\n", err, GetLastError(), WSAGetLastError());
    }
     //#define FILE_TYPE_UNKNOWN   0x0000    Either the type of the specified file is unknown, or the function failed.
     //#define FILE_TYPE_DISK      0x0001    The specified file is a disk file.
     //#define FILE_TYPE_CHAR      0x0002    The specified file is a character file, typically an LPT device or a console.
     //#define FILE_TYPE_PIPE      0x0003    The specified file is a socket, a named pipe, or an anonymous pipe.
     //#define FILE_TYPE_REMOTE    0x8000    Unused.
}

int dup_ex(HANDLE fd) {
    printf("dup(%p) %u\n", fd, GetFileType(fd));
    return (int)fd;
}

void set_inheritable(void* fdPtr, int inherit) {
    HANDLE fd = (HANDLE)fdPtr;
    SetHandleInformation(fd, HANDLE_FLAG_INHERIT, inherit);
    printf("set_inheritable(%p,%d) %d\n", fd, inherit, GetFileType(fd));
}

int socket_ex(int family){
    SOCKET ret = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("socket(%d) ret %llu\n", family, ret);
    return (int)ret;
}

#else
#define PLATFORM_IS_WINDOWS 0

int GetFileHandle(int fd) {
    return fd;
}

int GetStdIn(void){
    return 0;
}

int GetStdOut(void){
    return 1;
}

int GetStdErr(void){
    return 2;
}

int setsockopt_reuseport(int fd) {
    int reuseport_flag = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, (char*)&reuseport_flag, sizeof(reuseport_flag));
}

int is_sigchild(int sig) {
    return sig == SIGCHLD;
}
#endif

PyObject *
MakeUnixSockPyAddr(struct sockaddr_un *addr)
{
    if (addr->sun_family != AF_UNIX) {
        PyErr_SetString(
            PyExc_ValueError, "a UNIX socket addr was expected");
        return NULL;
    }

#ifdef __linux__
    int addrlen = sizeof (struct sockaddr_un);
    size_t linuxaddrlen = addrlen - offsetof(struct sockaddr_un, sun_path);
    if (linuxaddrlen > 0 && addr->sun_path[0] == 0) {
        return PyBytes_FromStringAndSize(addr->sun_path, linuxaddrlen);
    }
    else
#endif /* linux */
    {
        /* regular NULL-terminated string */
        return PyUnicode_DecodeFSDefault(addr->sun_path);
    }
}


#if PY_VERSION_HEX < 0x03070100

PyObject * Context_CopyCurrent(void) {
    return (PyObject *)PyContext_CopyCurrent();
};

int Context_Enter(PyObject *ctx) {
    return PyContext_Enter((PyContext *)ctx);
}

int Context_Exit(PyObject *ctx) {
    return PyContext_Exit((PyContext *)ctx);
}

#else

PyObject * Context_CopyCurrent(void) {
    return PyContext_CopyCurrent();
};

int Context_Enter(PyObject *ctx) {
    return PyContext_Enter(ctx);
}

int Context_Exit(PyObject *ctx) {
    return PyContext_Exit(ctx);
}

#endif
