"""Module for configuring a raw data file"""

import os

from LDMX.Framework import Processor, parameter_set, processor


@parameter_set
class RawDataFile:
    """RawDataFile configuration class"""

    filename: str = ""
    is_output: bool = False
    ecal_object_name: str = "EcalRaw"
    hcal_object_name: str = "HcalRaw"
    tracker_object_name: str = "TrackerRaw"
    triggerpad_object_name: str = "TriggerPadRaw"
    pass_name: str = ""
    skip_unavailable: bool = True
    ecal_object_passname_: str = ""
    hcal_object_passname_: str = ""
    triggerpad_object_passname_: str = ""
    tracker_object_event_passname_: str = ""

    @staticmethod
    def source(file_name):
        return RawDataFile(filename=file_name, is_output=False)

    @staticmethod
    def destination(file_name):
        return RawDataFile(filename=file_name, is_output=True)


@processor("packing::RawIO", "Packing")
class RawIO(Processor):
    """Producer which runs a single raw data file for input/output

    This producer does _nothing_ except pass handles to the RawDataFile
    for reading or writing. We are just wrapping a configuration class
    for the underlying raw data file, so we modify parameters via the
    raw_file member variable.

      rawinput = RawIO.source('my_data.raw')
      rawinput.raw_file.ecal_object_name = '2Ecal2Raw'
    """

    raw_file: RawDataFile = RawDataFile()

    @staticmethod
    def source(file_name):
        return RawIO(
            raw_file=RawDataFile.source(file_name), instance_name=f"IO_{file_name}"
        )

    @staticmethod
    def destination(file_name):
        return RawIO(
            raw_file=RawDataFile.destination(file_name), instance_name=f"IO_{file_name}"
        )


@processor("packing::SingleSubsystemUnpacker", "Packing")
class SingleSubsystemUnpacker(Processor):
    """Configuration for unpacking a single subsystem's raw data file
    into a series of vector buffers to put onto the event bus.

    Parameters
    ----------
    dat_file : str
        File path to raw data file to read in
    output_name : str
        Name of buffer object for event bus
    subsystem : int
        subsystem ID number to filter for (-1 if using subsystem_name)
    subsystem_name : str
        subsystem name to filter for (empty if using subsystem int)
    contributor : int
        contributor ID number to filter for (-1 means don't apply the filter)
    frame_offset : int
        number of frames for the subsystem to skip at the beginning of the file
    """

    dat_file: str = ""
    output_name: str = ""
    subsystem: int = -1
    subsystem_name: str = ""
    contributor: int = -1
    frame_offset: int = 0

    def __post_init__(self):
        if self.instance_name == self.__dataclass_fields__["instance_name"].default:
            self.instance_name = f"unpack_{os.path.basename(self.dat_file)}"


@processor("packing::SingleSubsystemPacker", "Packing")
class SingleSubsystemPacker(Processor):
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

    raw_file: str = ""
    input_name: str = ""
    input_pass: str = ""

    def __post_init__(self):
        if self.instance_name == self.__dataclass_fields__["instance_name"].default:
            self.instance_name = f"pack_{os.path.basename(self.raw_file)}"


@processor("packing::WRRawDecoder", "Packing", "wr")
class WRRawDecoder(Processor):
    input_file: str = ""
    output_name: str = ""
    ntuplize: bool = True


@processor("packing::FiberTrackerRawDecoder", "Packing")
class FiberTrackerRawDecoder(Processor):
    input_file: str = ""
    output_name: str = ""
    ntuplize: bool = True
