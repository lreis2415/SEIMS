#! /usr/bin/env python
# -*- coding: utf-8 -*-
"""Base classes of user defined tools for NSAG-II.
    @author   : Huiran Gao, Liangjun Zhu
    @changelog: 16-11-08  hr - initial implementation.\n
                17-08-18  lj - move the original code to slpposunits.\n
"""


# Initial tool functions supplemented to DEAP.tools


def initRepeatWithCfg(container, func, cf, n=2):
    """Call the function `container` with a generator function corresponding
    to the calling `n` times the function `func` with an argument `cf`.

    This function is an extension to the `DEAP.tools.initRepeat`.

    Args:
        container: The type to put in the data from `func`.
        func: The function that will be called n times to fill the `container`.
        cf: the only argument of `func`, which can be any instance.
        n: The number of times to repeat `func`.

    Returns:
        An instance of the container filled with data from func.
    """
    return container(func(cf) for _ in xrange(n))


def initIterateWithCfg(container, generator, cf=None):
    """ Call the function `container` with an iterable as
    its only argument. The iterable must be returned by
    the method or the object `generator` with one only
    or without argument

    This function can totally replace the `DEAP.tools.initIterate`.

    Args:
        container: The type to put in the data from `generator`.
        generator: A function returning an iterable (list, tuple, ...),
                      the content of this iterable will fill the container.
        cf: the only argument of `generator`, which can be any instance or None.

    Returns:
        An instance of the container filled with data from the `generator`.
    """
    if cf is None:
        return container(generator())
    else:
        return container(generator(cf))
