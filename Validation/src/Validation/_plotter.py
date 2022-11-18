"""Decorator for registering plotters

This is the wackiest python thing in this package and is
used to allow the CLI to have a list of all plotters in
submodules. In order for this to function, a plotting function
needs...

1. to be in a module imported in __init__.py. This is required
   so that the function is imported when the parent module is
   imported
2. to be decorated by the 'plotter' decorator below.

Attributes
----------
PLOTTERS : dict
    dictionary of plotters within Validation
"""

PLOTTERS = dict()

def plotter(hist = False, event = True) :
    """decorator for registering plotters

    There are three options for a plotter.

    1. Plots from histogram files
    2. Plots from event files
    3. Plots from both at the same time

    (1) and (2) have the same function signature but will
    be given a histogram-file or event-file Differ file (respectively).

    (3) has a longer signature for accepting both histogram- and event-
    file Differ objects at once.

    Examples
    --------
    Register a histogram-file plotter

        @plotter(hist=True,event=False)
        def my_hist_plotter(d, out_dir = None) :
            # d will be a Differ with histogram-files

    Register a event-file plotter

        @plotter(hist=False,event=True)
        def my_event_plotter(d, out_dir = None) :
            # d will be a Differ with event-files

    Register a plotter that can do both

        @plotter(hist=True,event=True)
        def plots_both(hd, ed, out_dir = None) :
            # hd will be histogram-files and ed will be event-files

    """
    if not hist and not event :
        raise ArgumentError('Need to plot one or both hist or event')
    def plotter_decorator(func) :
        func_name = func.__module__.replace('Validation.','')+'.'+func.__name__
        PLOTTERS[func_name] = (hist, event, func)
        return func
    return plotter_decorator
