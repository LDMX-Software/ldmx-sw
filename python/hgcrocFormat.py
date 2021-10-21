"""Decoding and Encoding HCal Digis out of and into raw buffers"""

from LDMX.Framework.ldmxcfg import Producer

class HcalRawDecoder(Producer) :
    """Decode a raw buffer into HCal Digis

    Parameters
    """

    def __init__(self, output_name, roc_version = 2, input_name = 'HcalRawData', num_packets_per_event = 1, input_file = '', connections_table = None) :
        super().__init__('hcalrawdecode','hcal::HcalRawDecoder','Hcal')

        self.input_name = input_name
        self.input_pass = ''
        self.input_file = input_file
        self.output_name = output_name
        self.roc_version = roc_version
        self.num_packets_per_event = num_packets_per_event

        if connections_table is None :
            self.translate_eid = False
        else :
            # load connections table into conditions system
            from LDMX.Hcal.DetectorMap import HcalDetectorMap
            HcalDetectorMap(connections_table)
            self.translate_eid = True
