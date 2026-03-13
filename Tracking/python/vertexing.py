from LDMX.Framework import Processor, field, processor

from .make_path import makeFieldMapPath


@processor("tracking::reco::VertexProcessor", "Tracking")
class VertexProcessor(Processor):
    """Producer to form vertices from a track collection.

    Currently, only vertex fitting has been implemented. Example use cases: K0
    and electronuclear studies.

    Attributes
    ----------
    field_map : str
        The path to the magnetic field map.
    trk_coll_name : str
        The name of the collection containing the tracks to vertex.
    input_pass_name : str
        The pass name of the input collections.
    seeds_coll_name : str
        The name of the seeds collection.
    """

    field_map: str = field(default_factory=makeFieldMapPath)
    trk_coll_name: str = "Tracks"
    seeds_coll_name: str = "RecoilTruthSeeds"
    input_pass_name: str = ""


@processor("tracking::reco::Vertexer", "Tracking")
class Vertexer(Processor):
    """Producer that forms vertices between two different track
    collections e.g. tagger and recoil tracks.

    Attributes
    ----------
    debug : bool
        Flag use to enable/disable printing of debug.
    field_map : str
        The path to the magnetic field map.
    trk_c_name_1 : str
        Name of a track collection to vertex.
    trk_c_name_2 : str
        Name of a track collection to vertex. This is unique from
        trk_c_name_1.
    input_pass_name : str
        The pass name of the input collections.
    """

    debug: bool = False
    field_map: str = field(default_factory=makeFieldMapPath)
    trk_c_name_1: str = "TaggerTracks"
    trk_c_name_2: str = "RecoilTracks"
    input_pass_name: str = ""
