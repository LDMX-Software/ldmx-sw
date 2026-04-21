"""Module for configuring the EventBuilder producer"""

from LDMX.Framework import ldmxcfg

class EventBuilder(ldmxcfg.Producer):
    """Configuration wrapper for the `eventbuilder::Builder` producer.

    Parameters
    ----------
    dat_file : str | None
        Path to the input raw dat file (optional).
    output_name : str
        Name for the output product placed on the event bus.
    verbose_parse : bool
        Enable verbose parsing output.
    coherence_window_ns : int
        Coherence window in nanoseconds for collecting fragments from the same physics event.
    instance_name : str
        Instance name given to the Producer.
    """

    def __init__(self, dat_file=None, output_name='PhysicsEventData', verbose_parse=True,
                 coherence_window_ns=5000000., instance_name='EventBuilder'):
        super().__init__(instance_name, 'eventbuilder::EventBuilder', 'EventBuilder')
        self.dat_file = dat_file or ''
        self.output_name = output_name
        self.verbose_parse = verbose_parse
        self.coherence_window_ns = coherence_window_ns


def from_dat_file(dat_file, output_name='PhysicsEventData', verbose_parse=True,
                  coherence_window_ns=5000000., instance_name=None):
    """Convenience factory to create a configured Builder.

    Example
    -------
    evb = Builder.from_dat_file('data.dat')
    p.sequence = [ evb ]
    """
    if instance_name is None:
        import os
        instance_name = f'EventBuilder_{os.path.basename(dat_file)}'
    return EventBuilder(dat_file=dat_file, output_name=output_name,
                        verbose_parse=verbose_parse, coherence_window_ns=coherence_window_ns,
                        instance_name=instance_name)
