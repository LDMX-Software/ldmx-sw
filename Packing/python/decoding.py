"""Configuration of decoders for raw data formats"""

from LDMX.Framework import Processor, processor


@processor("packing::ECONDDecoder", "Packing")
class ECONDDecoder(Processor):
    """Decode raw binary data assuming the ECON-D data format

    The input is the buffer that SingleSubsystemUnpacker puts on the event bus
    holding all of the samples from a single ECON-D. The channel samples are
    put into an HgcrocDigiCollection and the rest of the packet data (headers,
    common modes, calibration channel, corruption flags) into an
    ECONDEventInfo. Corrupted packets are flagged and skipped, the digis are
    only filled if all samples of the event decoded without problems.

    A summary of the decoding is logged at the information level at the end
    of processing, use ``p.logger.custom(decoder, level=1)`` to see it.

    Parameters
    ----------
    raw_data_name : str
        name of the buffer put on the bus by SingleSubsystemUnpacker
    raw_data_pass : str
        pass of the buffer (empty means any pass)
    digi_output_name : str
        name of the output HgcrocDigiCollection
    info_output_name : str
        name of the output ECONDEventInfo, defaults to
        digi_output_name + "ECONDInfo"
    is_ecal : bool
        True for ECal (EcalElectronicsID), False for HCal (HcalElectronicsID)
    n_links : int
        number of enabled eRx on the ECON-D
    fiber : int
        optical fiber index used in the electronics ID
    translate_eid : bool
        translate the electronics IDs into detector IDs using the
        detector map conditions object
    roc_version : int
        HGCROC version stored in the HgcrocDigiCollection
    """

    raw_data_name: str = None
    raw_data_pass: str = ""
    digi_output_name: str = None
    info_output_name: str = ""
    is_ecal: bool = None
    n_links: int = 2
    fiber: int = 0
    translate_eid: bool = False
    roc_version: int = 3

    def __post_init__(self):
        if self.raw_data_name is None:
            raise ValueError(
                "ECONDDecoder requires raw_data_name to know where to look for data!"
            )
        if self.digi_output_name is None:
            raise ValueError(
                "ECONDDecoder requires digi_output_name to know where to put the "
                "decoded data!"
            )
        if self.is_ecal is None:
            raise ValueError(
                "ECONDDecoder requires is_ecal to know which subsystem it is decoding!"
            )
        if not self.info_output_name:
            self.info_output_name = f"{self.digi_output_name}ECONDInfo"
        if self.instance_name == self.__dataclass_fields__["instance_name"].default:
            self.instance_name = f"decode_{self.raw_data_name}"
