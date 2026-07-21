"""Configuration for Cluster Triplet text file Maker and Pattern LUT Maker"""

from LDMX.Framework import Processor, processor


@processor("tools::ClusterTripletMaker", "Tools")
class ClusterTripletMaker(Processor):
    """Configuration for cluster text file maker for Trigger Scintillators"""

    cluster_input_collections: list[str] = [
        "TriggerPad1Clusters",
        "TriggerPad2Clusters",
        "TriggerPad3Clusters",
    ]
    pass_name: str = ""
    output_file: str = "clusters.txt"
    verbosity: int = 0


@processor("tools::PatternLUTMaker", "Tools")
class PatternLUTMaker(Processor):
    """Configuration for track-pattern LUT-writing analyzer for Trigger Scintillators"""

    input_file: str = "clusters.txt"
    input_pass_name: str = ""
    output_file: str = "LUT.txt"
    lut_threshold: float = 0.0008
    verbosity: int = 0
