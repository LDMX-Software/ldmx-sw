"""Decoding and Encoding HCal Digis out of and into raw buffers"""

from LDMX.Framework import Processor, field, processor


@processor("hcal::HcalRawDecoder", "Hcal")
class HcalRawDecoder(Processor):
    output_name: str
    roc_version: int = 2
    input_pass: str = ""
    detector_name: str = "ldmx-hcal-prototype-v1.0"
    input_names: list[str] | None = None
    input_file: str | None = None
    read_from_file: bool = field(init=False)
    connections_table: str | None = None

    def __post_init__(self):
        if self.input_names is not None:
            self.read_from_file = False
            self.input_file = ""
        elif self.input_file is not None:
            self.read_from_file = True
            self.input_names = [""]
        else:
            raise Exception("Must read from event bus or input file.")

        from LDMX.Framework import ldmxcfg

        from .detector_map import HcalDetectorMap

        if self.connections_table is None:
            # attempt to deduce if using eid based on presence of
            # HcalDetectorMap in conditions system
            self.translate_eid = False
            for cop in ldmxcfg.Process.last_process.conditions_object_providers:
                if isinstance(cop, HcalDetectorMap):
                    self.translate_eid = True
                    break
        else:
            # load connections table into conditions system
            HcalDetectorMap(self.connections_table)
            self.translate_eid = True


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
