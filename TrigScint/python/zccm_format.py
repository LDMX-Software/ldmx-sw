"""TrigScint.ZCCMEncoder Python module

Sets all parameters to reasonable defaults.

Examples
--------
from LDMX.TrigScint.zccmFormat import ZCCMDecoder
dec=ZCCMDecoder.tagger("$LDMX_BASE/ldmx-sw/TrigScint/util/channelMapFrontBack.txt")
p.sequence.extend([ dec ])

"""

# NOTE this needs rewriting substantially to deal with the fact that
# we're doing all modules in one stream

from LDMX.Framework import Processor, processor


@processor("trigscint::ZCCMEncoder", "TrigScint")
class ZCCMEncoder(Processor):
    """Configuration for ZCCM encoder"""

    channel_map_file: str = ""
    module_map_file: str = ""
    input_pass_name: str = ""
    input_collection: str = "trigScintZCCMDigis"
    output_collection: str = "ZCCMDigis"
    number_channels: int = 84

    @staticmethod
    def tagger(map_file, **kwargs):
        """Get the encoding emulator for the trigger pad upstream of tagger"""
        return ZCCMEncoder(
            channel_map_file=map_file,
            instance_name="tag",
            input_collection="trigScintZCCMDigisTag",
            output_collection="ZCCMstreamTag",
            **kwargs,
        )

    @staticmethod
    def up(map_file, **kwargs):
        """Get the encoding emulator for the trigger pad upstream of target"""
        return ZCCMEncoder(
            channel_map_file=map_file,
            instance_name="up",
            input_collection="trigScintZCCMDigisUp",
            output_collection="ZCCMstreamUp",
            **kwargs,
        )

    @staticmethod
    def down(map_file, **kwargs):
        """Get the encoding emulator for the trigger pad downstream of target"""
        return ZCCMEncoder(
            channel_map_file=map_file,
            instance_name="down",
            input_collection="trigScintZCCMDigisDown",
            output_collection="ZCCMstreamDown",
            **kwargs,
        )


@processor("trigscint::ZCCMDecoder", "TrigScint")
class ZCCMDecoder(Processor):
    """Configuration for ZCCM decoder"""

    channel_map_file: str = ""
    module_map_file: str = ""
    input_pass_name: str = ""
    input_collection: str = "ZCCMoutput"
    output_collection: str = "decodedZCCM"
    number_channels: int = 84
    number_time_samples: int = 70
    is_real_data: bool = True

    @staticmethod
    def tagger(map_file, **kwargs):
        """Get the decoding emulator for the trigger pad upstream of tagger"""
        return ZCCMDecoder(
            channel_map_file=map_file,
            instance_name="tag",
            input_collection="ZCCMstreamTag",
            output_collection="decodedZCCMTag",
            **kwargs,
        )

    @staticmethod
    def up(map_file, **kwargs):
        """Get the decoding emulator for the trigger pad upstream of target"""
        return ZCCMDecoder(
            channel_map_file=map_file,
            instance_name="up",
            input_collection="ZCCMoutputUp",
            output_collection="decodedZCCMUp",
            **kwargs,
        )

    @staticmethod
    def down(map_file, **kwargs):
        """Get the decoding emulator for the trigger pad downstream of target"""
        return ZCCMDecoder(
            channel_map_file=map_file,
            instance_name="down",
            input_collection="ZCCMstreamDown",
            output_collection="decodedZCCMDown",
            **kwargs,
        )
