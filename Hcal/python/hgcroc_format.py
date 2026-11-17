"""Processors for HCal test beam raw data"""

from LDMX.Framework import Processor, processor


@processor("hcal::HcalAlignPolarfires", "Hcal")
class HcalAlignPolarfires(Processor):
    """Align the two polarfires from testbeam into singular events

    Parameters
    ----------
    output_name : str
        Name of event object with aligned digis
    input_names : list[str]
        The event objects of the decoded digis from the two polarfires
    input_pass : str
        pass generating decoded digis
    max_tick_diff : int
        Maximum number of ticks to consider the two polarfires on the same event
    drop_lonely_events : bool
        *REMOVED* - replace with direct call to p.skim_consider
    keep_inputs : bool
        *REMOVED* - replace with direct extension of p.keep for the input_names
    """

    output_name: str
    input_names: list[str]
    input_pass: str = ""
    max_tick_diff: int = 10
