"""Lazy-loading models module to avoid tensorflow import errors."""

def __getattr__(name):
    if name == 'MultiOutputANN':
        from .multi_output_ann import MultiOutputANN
        return MultiOutputANN
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

__all__ = ['MultiOutputANN']
