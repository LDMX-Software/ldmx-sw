"""Decoding and Encoding HCal Digis out of and into raw buffers"""

from LDMX.Framework.ldmxcfg import Producer

class HcalRawDecoder(Producer) :
    """Decode a raw buffer into HCal Digis

    Parameters
    """

    def __init__(self, output_name, roc_version = 2, 
            input_names = None, input_pass = '',
            input_file = None, connections_table = None, 
            detector_name = 'ldmx-hcal-prototype-v1.0') :
        super().__init__('hcalrawdecode','hcal::HcalRawDecoder','Hcal')

        if input_names is not None :
            self.read_from_file = False
            self.input_file = ''
            self.input_names = input_names
        elif input_file is not None :
            self.read_from_file = True
            self.input_file = input_file
            self.input_names = ['']
        else :
            raise Exception("Must read from event bus or input file.")

        self.input_pass = input_pass # only used when reading from event bus
        self.output_name = output_name
        self.roc_version = roc_version
        self.detector_name = detector_name # only used when reading from file

        from LDMX.Framework import ldmxcfg
        from LDMX.Hcal.DetectorMap import HcalDetectorMap
        if connections_table is None :
            # deduce if using eid based on presence of HcalDetectorMap in conditions system
            self.translate_eid = False
            for cop in ldmxcfg.Process.lastProcess.conditionsObjectProviders :
                if isinstance(cop,HcalDetectorMap) :
                    self.translate_eid = True
                    break
        else :
            # load connections table into conditions system
            HcalDetectorMap(connections_table)
            self.translate_eid = True

class HcalAlignPolarfires(Producer) :
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
        True if you want to drop events that only have data from one polarfire
    keep_inputs : bool
        True if you want to keep both copies of the decoded data,
        effectively doubles the size of the output file, only use in debugging/developing
    """

    def __init__(self, output_name, input_names, input_pass = '',
            max_tick_diff = 10, drop_lonely_events = False, keep_inputs = False) :
        super().__init__('hcalalign','hcal::HcalAlignPolarfires','Hcal')

        self.output_name = output_name
        self.input_names = input_names
        self.input_pass = input_pass
        self.max_tick_diff = max_tick_diff

        from LDMX.Framework import ldmxcfg
        p = ldmxcfg.Process.lastProcess

        if drop_lonely_events :
            p.skimConsider(self.name)

        if not keep_inputs :
            for i in self.input_names :
                p.keep.append(f'drop {i}.*')

