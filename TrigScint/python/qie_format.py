"""TrigScint.QIEEncoder Python module

Sets all parameters to reasonable defaults.

Examples
--------
from LDMX.TrigScint.qieFormat import QIEDecoder
dec=QIEDecoder.tagger("$LDMX_BASE/ldmx-sw/TrigScint/util/channelMapFrontBack.txt")
p.sequence.extend([ dec ])

"""

from LDMX.Framework import Processor, processor


@processor("trigscint::QIEEncoder", "TrigScint")
class QIEEncoder(Processor):
    """Configuration for QIE encoder"""

    channel_map_file: str = ""
    input_pass_name: str = ""
    input_collection: str = "trigScintQIEDigisTag"
    output_collection: str = "QIEstreamTag"
    number_channels: int = 50
    verbose: bool = False

    @staticmethod
    def tagger(map_file, **kwargs):
        """Get the encoding emulator for the trigger pad upstream of tagger"""
        return QIEEncoder(
            channel_map_file=map_file,
            instance_name="tag",
            input_collection="trigScintQIEDigisTag",
            output_collection="QIEstreamTag",
            **kwargs,
        )

    @staticmethod
    def up(map_file, **kwargs):
        """Get the encoding emulator for the trigger pad upstream of target"""
        return QIEEncoder(
            channel_map_file=map_file,
            instance_name="up",
            input_collection="trigScintQIEDigisUp",
            output_collection="QIEstreamUp",
            **kwargs,
        )

    @staticmethod
    def down(map_file, **kwargs):
        """Get the encoding emulator for the trigger pad downstream of target"""
        return QIEEncoder(
            channel_map_file=map_file,
            instance_name="down",
            input_collection="trigScintQIEDigisDown",
            output_collection="QIEstreamDown",
            **kwargs,
        )


@processor("trigscint::QIEDecoder", "TrigScint")
class QIEDecoder(Processor):
    """Configuration for QIE decoder"""

    channel_map_file: str = ""
    input_pass_name: str = ""
    input_collection: str = "QIEstreamTag"
    output_collection: str = "decodedQIETag"
    number_channels: int = 50
    number_time_samples: int = 5
    is_real_data: bool = False
    verbose: bool = False

    @staticmethod
    def tagger(map_file, **kwargs):
        """Get the decoding emulator for the trigger pad upstream of tagger"""
        return QIEDecoder(
            channel_map_file=map_file,
            instance_name="tag",
            input_collection="QIEstreamTag",
            output_collection="decodedQIETag",
            **kwargs,
        )

    @staticmethod
    def up(map_file, **kwargs):
        """Get the decoding emulator for the trigger pad upstream of target"""
        return QIEDecoder(
            channel_map_file=map_file,
            instance_name="up",
            input_collection="QIEstreamUp",
            output_collection="decodedQIEUp",
            **kwargs,
        )

    @staticmethod
    def down(map_file, **kwargs):
        """Get the decoding emulator for the trigger pad downstream of target"""
        return QIEDecoder(
            channel_map_file=map_file,
            instance_name="down",
            input_collection="QIEstreamDown",
            output_collection="decodedQIEDown",
            **kwargs,
        )
