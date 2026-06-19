"""Configuration for Cluster Triplet text file Maker and Pattern LUT Maker"""

from LDMX.Framework import parameter_set

@parameter_set
class ClusterTripletMaker :
    """Configuration for cluster text file maker for Trigger Scintillators"""
    cluster_input_collections: list[str] = ["TriggerPad1Clusters", 
                                            "TriggerPad2Clusters","TriggerPad3Clusters"]
    pass_name: str = ""
    output_collection: str = "clusters.txt"
    verbosity: int = 0


@parameter_set
class PatternLUTMaker :
    """Configuration for track-pattern LUT-writing analyzer for Trigger Scintillators"""

    input_collection: str ="clusters.txt"
    input_pass_name: str = ""
    output_collection: str ="LUT.txt"
    lut_threshold: float = 0.0008
    verbosity: int = 0