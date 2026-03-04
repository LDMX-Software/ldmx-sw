"""Plotting of performance plots"""

import logging

from ._differ import Differ
from ._plotter import plotter


log = logging.getLogger('performance')

@plotter
def event_timing(d : Differ, out_dir = None) :
    """Plot time it took to process events

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    # just plotting the __ALL__ branch which represents all processors in the sequence,
    # however, if we introduce some introspection into the Differ class, we could deduce
    # the other branch names and plot the duration of different processors specifically
    for processor in ['__ALL__']:
        branch = f'performance/by_event/{processor}./{processor}.duration_'
        log.info(f'plotting event time for {processor}')
        d.plot1d(
            branch,
            f'{processor} Event Time [s]',
            out_dir=out_dir,
            legend_kw=dict(loc='upper right')
        )
