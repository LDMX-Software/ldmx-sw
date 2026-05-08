
from LDMX.Framework import Processor, parameter_set, processor

@processor("packing::ECONDDecoder", "Packing")
class ECONDDecoder(Processor):
    """decode raw binary data assuming ECON-D data format"""

    raw_data_name: str = None
    digi_output_name: str = None
    is_ecal: bool = None
    raw_data_pass: str = ""

    def __post__(self):
        if self.raw_data_name is None:
            raise ValueError('ECONDDecoder requires raw_data_name to know where to look for data!')
        if self.digi_output_name is None:
            raise ValueError('ECONDDecoder requires digi_output_name to know where to put the decoded data!')
        if self.is_ecal is None:
            raise ValueError('ECONDDecoder requires is_ecal to know which subsystem it is decoding!')
        if self.instance_name == self.__dataclass_fields__["instance_name"].default:
            self.instance_name = f"decode_{self.raw_data_name}"

