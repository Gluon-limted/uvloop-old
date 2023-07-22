import asyncio as __asyncio
import typing as _typing

from asyncio.events import BaseDefaultEventLoopPolicy as __BasePolicy

from . import includes as __includes  # NOQA
from .loop import Loop as __BaseLoop  # NOQA
from ._version import __version__  # NOQA


__all__ = ('new_event_loop', 'install', 'EventLoopPolicy')


class Loop(__BaseLoop, __asyncio.AbstractEventLoop):  # type: ignore[misc]
    pass


def new_event_loop() -> Loop:
    """Return a new event loop."""
    return Loop()


def install() -> None:
    """A helper function to install uvloop policy."""
    __asyncio.set_event_loop_policy(EventLoopPolicy())


class EventLoopPolicy(__BasePolicy):
    """Event loop policy.

    The preferred way to make your application use uvloop:

    >>> import asyncio
    >>> import uvloop
    >>> asyncio.set_event_loop_policy(uvloop.EventLoopPolicy())
    >>> asyncio.get_event_loop()
    <uvloop.Loop running=False closed=False debug=False>
    """

    def _loop_factory(self) -> Loop:
        return new_event_loop()

    if _typing.TYPE_CHECKING:
        # EventLoopPolicy doesn't implement these, but since they are marked
        # as abstract in typeshed, we have to put them in so mypy thinks
        # the base methods are overridden. This is the same approach taken
        # for the Windows event loop policy classes in typeshed.
        def get_child_watcher(self) -> _typing.NoReturn:
            ...

        def set_child_watcher(
            self, watcher: _typing.Any
        ) -> _typing.NoReturn:
            ...

import functools
import time
from typing import Callable, Any
def async_timed():
    def wrapper(func: Callable) -> Callable:
        @functools.wraps(func)
        async def wrapped(*args, **kwargs) -> Any:
            print(f'starting {func} with args {args} {kwargs}')
            start = time.time()
            try:
                return await func(*args, **kwargs)
            finally:
                end = time.time()
                total = end - start
                print(f'finished {func} in {total:.4f} second(s)')

        return wrapped

    return wrapper

import sys
if sys.platform in ('win32', 'cli'):
    import sys, os.path, ctypes, ctypes.wintypes

    Psapi = ctypes.WinDLL('Psapi.dll')
    EnumProcesses = Psapi.EnumProcesses
    EnumProcesses.restype = ctypes.wintypes.BOOL
    GetProcessImageFileName = Psapi.GetProcessImageFileNameA
    GetProcessImageFileName.restype = ctypes.wintypes.DWORD

    Kernel32 = ctypes.WinDLL('kernel32.dll')
    OpenProcess = Kernel32.OpenProcess
    OpenProcess.restype = ctypes.wintypes.HANDLE
    TerminateProcess = Kernel32.TerminateProcess
    TerminateProcess.restype = ctypes.wintypes.BOOL
    CloseHandle = Kernel32.CloseHandle

    MAX_PATH = 260
    PROCESS_TERMINATE = 0x0001
    PROCESS_QUERY_INFORMATION = 0x0400

    def ProcessCount() :
        count = 32
        while True:
            ProcessIds = (ctypes.wintypes.DWORD*count)()
            cb = ctypes.sizeof(ProcessIds)
            BytesReturned = ctypes.wintypes.DWORD()
            if EnumProcesses(ctypes.byref(ProcessIds), cb, ctypes.byref(BytesReturned)):
                if BytesReturned.value<cb:
                    break
                else:
                    count *= 2
            else:
                sys.exit("Call to EnumProcesses failed")
        return BytesReturned.value / ctypes.sizeof(ctypes.wintypes.DWORD), ProcessIds

    def IsProcessRunning(pid):
        processCount, ProcessIds = ProcessCount()
        return pid in ProcessIds
    def KillProcessName(processname):
        processCount, ProcessIds = ProcessCount()
        for index in range(processCount):
            ProcessId = ProcessIds[index]
            hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, False, ProcessId)
            if hProcess:
                ImageFileName = (ctypes.c_char*MAX_PATH)()
                if GetProcessImageFileName(hProcess, ImageFileName, MAX_PATH)>0:
                    filename = os.path.basename(ImageFileName.value)
                    if filename == processname:
                        TerminateProcess(hProcess, 1)
                CloseHandle(hProcess)
else:
    def IsProcessRunning(pid):
        try :
            os.kill(pid, 0)
            return True
        except ProcessLookupError:
            return False
        except:
            raise

