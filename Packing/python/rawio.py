"""Module for configuring a raw data file"""

from LDMX.Framework import ldmxcfg
import os

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

class SingleSubsystemUnpacker(ldmxcfg.Producer) :
    """Configuration for unpacking a single subsystem's raw data file
    into a series of vector buffers to put onto the event bus.

    Parameters
    ----------
    raw_file : str
        File path to raw data file to read in
    output_name : str
        Name of buffer object for event bus
    num_words_per_event : int
        Number of words to put onto each event bus
    bytes_per_word : int
        Number of bytes in each word of buffer (1,2,4, or 8)
    detector_name : str
        Name of the detector GDML file
    """

    def __init__(self, raw_file, output_name, num_words_per_event,detector_name, bytes_per_word = 1) :
        super().__init__(f'unpack_{os.path.basename(raw_file)}','packing::SingleSubsystemUnpacker','Packing')
        self.raw_file = raw_file
        self.output_name = output_name
        self.num_words_per_event = num_words_per_event
        self.num_bytes_per_word = bytes_per_word
        self.detector_name = detector_name

class SingleSubsystemPacker(ldmxcfg.Analyzer) :
    """Configuration for packing a single subsystem's encoded buffer
    into a raw data file in sequence.

    Parameters
    ----------
    raw_file : str
        File path to raw data file to write out
    input_name : str
        event bus object name for encoded buffer
    input_pass : str
        event bus object pass for encoded buffer
    bytes_per_word : int
        Number of bytes to in each word of buffer
    """

    def __init__(self, raw_file, input_name, input_pass = '', bytes_per_word = 1) :
        super().__init__(f'pack_{os.path.basename(raw_file)}','packing::SingleSubsystemPacker','Packing')
        self.raw_file = raw_file
        self.input_name = input_name
        self.input_pass = input_pass
        self.num_bytes_per_word = bytes_per_word
