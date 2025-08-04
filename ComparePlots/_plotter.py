"""Decorator for registering plotters

This is the wackiest python thing in this package and is
used to allow the CLI to have a list of all plotters in
submodules. In order for this to function, a plotting function
needs...

1. to be in a module imported in __init__.py. This is required
   so that the function is imported when the parent module is
   imported
2. to be decorated by the 'plotter' decorator below.
"""

def plotter(func):
    """decorator for registering plotters

    Examples
    --------
    Register a plotter

        @plotter
        def my_hist_plotter(d, out_dir = None) :
            # d will be a Differ with histogram-files

    Attributes
    ----------
    __registry__ : dict
        Dictionary of registered plotters
    """
    func_name = func.__module__.replace('Validation.','')+'.'+func.__name__
    plotter.__registry__[func_name] = func
    return func


plotter.__registry__ = dict()
