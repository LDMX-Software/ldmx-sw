"""Decoding and Encoding HCal Digis out of and into raw buffers"""

from LDMX.Framework.ldmxcfg import Producer

class HcalRawDecoder(Producer) :
    """Decode a raw buffer into HCal Digis

    Parameters
    """

    def __init__(self, output_name, roc_version = 2, 
            input_name = None, input_pass = '',
            input_file = None, connections_table = None, 
            detector_name = 'ldmx-hcal-prototype-v1.0') :
        super().__init__('hcalrawdecode','hcal::HcalRawDecoder','Hcal')

        if input_name is not None :
            self.read_from_file = False
            self.input_name = input_name
        elif input_file is not None :
            self.read_from_file = True
            self.input_name = input_file
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
