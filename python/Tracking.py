from LDMX.Framework.ldmxcfg import Producer

class SeedFinderProcessor(Producer) :
    def __init__(self, instance_name = "SeedFinderProcessor"):
        super().__init__(instance_name, 'tracking::sim::SeedFinderProcessor','Tracking')

class CKFProcessor(Producer) : 
  def __init__(self, instance_name = 'CKFProcessor'): 
    super().__init__(instance_name, 'tracking::sim::CKFProcessor', 'Tracking')

class TruthSeedProcessor(Producer) :
    def __init__(self, instance_name = "TruthSeedProcessor"):
        super().__init__(instance_name, 'tracking::reco::TruthSeedProcessor','Tracking')

#This class produces vertices from a track collection, i.e. could be used for K0 analysis for EN scattering.
class VertexProcessor(Producer) :

    def __init__(self, instance_name = "VertexProcessor"):
        super().__init__(instance_name, 'tracking::reco::VertexProcessor','Tracking')

#This class is to produce vertices between two track collections, i.e. for tagger/recoil matching for example.
class Vertexer(Producer) :
    def __init__(self, instance_name = "Vertexer"):
        super().__init__(instance_name,'tracking::reco::Vertexer','Tracking')



