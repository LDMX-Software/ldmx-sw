"""Module for configuring the EventBuilder producer"""

from LDMX.Framework import Processor, processor


@processor("eventbuilder::EventBuilder", "EventBuilder")
class EventBuilder(Processor):
    """Configuration for the `eventbuilder::EventBuilder` producer.

    Attributes
    ----------
    dat_file : str
        Path to the input raw dat file.
    output_name : str
        Name for the output product placed on the event bus.
    verbose_parse : bool
        Enable verbose parsing output.
    coherence_window_ns : float
        Coherence window in nanoseconds for collecting fragments from the same
        physics event.
    min_subsystems : int
        Minimum number of distinct subsystems required within the coherence
        window to assemble an event. Default 2 (e.g. Run 182 has only ts +
        tracker); set higher for runs that read out more subsystems.
    """

    dat_file: str = ""
    output_name: str = "PhysicsEventData"
    verbose_parse: bool = True
    coherence_window_ns: float = 5000000.0
    min_subsystems: int = 2


def from_dat_file(
    dat_file,
    output_name="PhysicsEventData",
    verbose_parse=True,
    coherence_window_ns=5000000.0,
    min_subsystems=2,
    instance_name=None,
):
    """Convenience factory to create a configured EventBuilder.

    Example
    -------
    evb = from_dat_file('data.dat')
    p.sequence = [ evb ]
    """
    import os

    if instance_name is None:
        instance_name = f"EventBuilder_{os.path.basename(dat_file)}"
    evb = EventBuilder(instance_name=instance_name)
    evb.dat_file = dat_file
    evb.output_name = output_name
    evb.verbose_parse = verbose_parse
    evb.coherence_window_ns = coherence_window_ns
    evb.min_subsystems = min_subsystems
    return evb
