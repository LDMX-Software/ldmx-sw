"""Module for configuring a raw data file"""

from LDMX.Framework import ldmxcfg

class RawDataFile() :
    """RawDataFile configuration class"""

    def __init__(self, name, is_output) :
        self.filename = name
        self.is_output = is_output
        self.ecal_object_name = "EcalRaw"
        self.hcal_object_name = "HcalRaw"
        self.tracker_object_name = "TrackerRaw"
        self.triggerpad_object_name = "TriggerPadRaw"
        self.pass_name = ""
        self.skip_unavailable = True

class RawIO(ldmxcfg.Producer) :
    """Producer which runs a single raw data file for input/output

    This producer does _nothing_ except pass handles to the RawDataFile
    for reading or writing. We are just wrapping a configuration class
    for the underlying raw data file, so we modify parameters via the
    raw_file member variable.

      rawinput = RawIO.source('my_data.raw')
      rawinput.raw_file.ecal_object_name = '2Ecal2Raw'
    """

    def __init__(self, raw_file) :
        super().__init__(f'IO_{raw_file}','packing::RawIO','Packing')
        self.raw_file = raw_file

    def source(file_name) :
        return RawIO(RawDataFile(file_name, False))

    def destination(file_name) :
        return RawIO(RawDataFile(file_name, True))
