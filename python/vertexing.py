
from LDMX.Framework.ldmxcfg import Producer
from LDMX.Tracking.make_path import makeFieldMapPath

class VertexProcessor(Producer) :
    """ Producer to form vertices from a track collection.

    Currently, only vertex fitting has been implemented. Example use cases: K0 
    and electronuclear studies.

    Attributes
    ----------
    field_map : str
        The path to the magnetic field map.
    trk_coll_name: str
        The name of the collection containing the tracks to vertex.

    Parameters
    ----------
    name : str
        Unique name for this instance.
    """

    def __init__(self, name : str = "VertexProcessor"):
        super().__init__(name, 'tracking::reco::VertexProcessor','Tracking')

        self.field_map = makeFieldMapPath()
        self.trk_coll_name = 'Tracks'

class Vertexer(Producer) :
    """ Producer that forms vertices betwen two different track 
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

    Parameters
    ----------
    name : str
        Unique name for this instance.
    """
    def __init__(self, name : str = "Vertexer"):
        super().__init__(name,'tracking::reco::Vertexer','Tracking')

        self.debug = False
        self.field_map = makeFieldMapPath()
        trk_c_name_1 = 'TaggerTracks'
        trk_c_name_2 = 'RecoilTracks'
