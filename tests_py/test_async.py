
from .app_utils_test import make_simple_struct_future, SimpleStruct

import asyncio


def test_future():

    future = make_simple_struct_future()
    print()
    print(future)
    val = future.get()
    print(val)
    assert(isinstance(val, SimpleStruct))


class coro:
    def __init__(self):
        self._state = 0

    def __iter__(self):
        return self

    def __await__(self):
        return self

    def __next__(self):
        if self._state == 0:
            self._x = foo()
            self._bar_iter = bar().__await__()
            self._state = 1

        if self._state == 1:
            try:
                suspend_val = next(self._bar_iter)
                # propagate the suspended value to the caller
                # don't change _state, we will return here for
                # as long as bar() keeps suspending
                return suspend_val
            except StopIteration as stop:
                # we got our value
                y = stop.value
            # since we got the value, immediately proceed to
            # invoking `baz`
            baz(self._x, y)
            self._state = 2
            # tell the caller that we're done and inform
            # it of the return value
            raise StopIteration(42)

        # the final state only serves to disable accidental
        # resumption of a finished coroutine
        raise RuntimeError("cannot reuse already awaited coroutine")


def _test_coro():
    """
    see https://stackoverflow.com/a/54675255/4249338
    """
    print(asyncio.run(coro()))
