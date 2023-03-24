"""Base classes of user defined tools for NSAG-II.

    @author   : Huiran Gao, Liangjun Zhu

    @changelog:

    - 16-11-08  - hr - initial implementation.
    - 17-08-18  - lj - move the original code to spatialunits module.
    - 18-02-09  - lj - compatible with Python3.
"""
from __future__ import absolute_import, unicode_literals


# Initial tool functions supplemented to DEAP.tools

def initRepeatWithCfgFromList(container, func, cf, inlist):
    return container(func(cf, l) for l in inlist)


def initIterateWithCfgWithInput(container, generator, cf, input_genes):
    return container(generator(cf, input_genes))


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
    return container(func(cf) for _ in range(n))


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


def initRepeatWithCfgIndv(container, func, cf, indv, n=2):
    return container(func(cf, indv) for _ in range(n))


def initIterateWithCfgIndv(container, generator, cf, indv):
    if cf is None:
        return container(generator(indv))
    else:
        return container(generator(cf, indv))


def initIterateWithCfgIndvInput(container, generator, cf, indv):
    return container(generator(cf, indv, True))
