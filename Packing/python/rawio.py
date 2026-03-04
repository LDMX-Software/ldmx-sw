"""Module for configuring a raw data file"""

import os

from LDMX.Framework import ldmxcfg


class RawDataFile :
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

        self.ecal_object_passname_ = ''
        self.hcal_object_passname_ = ''
        self.triggerpad_object_passname_ = ''
        self.tracker_object_event_passname_ = ''

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
    @staticmethod
    def source(raw_file) :
        """Configure a RawIO producer for reading from a raw data file

        Parameters
        ----------
        raw_file : str
            File path to raw data file to read in
        """
        return RawIO(RawDataFile(raw_file, False))
    
    @staticmethod
    def destination(raw_file) :
        """Configure a RawIO producer for writing to a raw data file

        Parameters
        ----------
        raw_file : str
            File path to raw data file to write to
        """
        return RawIO(RawDataFile(raw_file, True))


class SingleSubsystemUnpacker(ldmxcfg.Producer) :
    """Configuration for unpacking a single subsystem's raw data file
    into a series of vector buffers to put onto the event bus.

    Parameters
    ----------
    dat_file : str
        File path to raw data file to read in
    output_name : str
        Name of buffer object for event bus
    subsystem : int
        subsystem ID number to filter for
    contributor : int
        contributor ID number to filter for (-1 means don't apply the filter)
    frame_offset : int
        number of frames for the subsystem to skip at the beginnig of the file
    """

    def __init__(self, dat_file, output_name, subsystem, contributor = -1, frame_offset = 0) :
        super().__init__(f'unpack_{os.path.basename(dat_file)}','packing::SingleSubsystemUnpacker','Packing')
        self.dat_file = dat_file
        self.output_name = output_name
        if type(subsystem) is str:
            self.subsystem_name = subsystem
            self.subsystem = -1
        else:
            self.subsystem_name = ''
            self.subsystem = subsystem
        self.contributor = contributor
        self.frame_offset = frame_offset


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
    """

    def __init__(self, raw_file, input_name, input_pass = '') :
        super().__init__(f'pack_{os.path.basename(raw_file)}','packing::SingleSubsystemPacker','Packing')
        self.raw_file = raw_file
        self.input_name = input_name
        self.input_pass = input_pass

class WRRawDecoder(ldmxcfg.Producer) :
    def __init__(self, raw_file, output_name, ntuplize = True, name = 'wr') :
        super().__init__(name,'packing::WRRawDecoder','Packing')
        self.input_file = raw_file
        self.output_name = output_name
        self.ntuplize = ntuplize

class FiberTrackerRawDecoder(ldmxcfg.Producer) :
    def __init__(self, raw_file, output_name, name, ntuplize = True) :
        super().__init__(name,'packing::FiberTrackerRawDecoder','Packing')
        self.input_file = raw_file
        self.output_name = output_name
        self.ntuplize = ntuplize
