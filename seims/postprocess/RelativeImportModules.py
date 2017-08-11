import importlib
import sys

from pathlib2 import Path


# Supplement for http://stackoverflow.com/a/28154841/2301450
# Generic boilerplate code for setting __package__ attribute for relative imports
# Code from https://gist.github.com/vaultah/d63cb4c86be2774377aa674b009f759a
# Usage:
#     1_level_example
#
#         if __name__ == '__main__' and __package__ is None:
#             __package__ import_parents()
#
#         from . import module
#         from .module.submodule import thing
#     3_level_example
#
#         if __name__ == '__main__' and __package__ is None:
#             __package__ = import_parents(level=3)
#
#         from ... import module
#         from ...module.submodule import thing

def import_parents(level=1):
    global __package__
    file = Path(__file__).resolve()
    parent, top = file.parent, file.parents[level]
    __package__ = '.'.join(parent.parts[len(top.parts):])
    sys.path.append(str(top))
    importlib.import_module(__package__)  # won't be needed after that
    return __package__


if __name__ == '__main__' and __package__ is None:
    import_parents(level=1)
