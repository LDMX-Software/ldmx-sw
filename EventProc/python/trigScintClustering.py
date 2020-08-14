"""Configuration for Trigger Scintillator cluster producer 

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.EventProc.trigScintClustering import TrigScintClusterProducer
    p.sequence.extend([ TrigScintClusterProducer.up() , TrigScintClusterProducer.down() , TrigScintClusterProducer.tagger() ])
"""

from LDMX.Framework import ldmxcfg

class TrigScintClusterProducer(ldmxcfg.Producer) :
    """Configuration for cluster producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintClusterProducer')

        from LDMX.EventProc import include
        include.library()

        self.max_cluster_width = 2
        self.clustering_threshold = 0.  #to add in neighboring channels
        self.seed_threshold = 30.
        self.input_collection="trigScintDigisTag"
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTaggerClusters"
        self.verbosity = 0

    def up() :
        """Get the cluster producer for the trigger pad upstream of target"""
        cluster = TrigScintClusterProducer( 'trigScintClustersUp' )
        cluster.input_collection = 'trigScintDigisUp'
        cluster.output_collection= 'TriggerPadUpClusters'
        return cluster

    def down() :
        """Get the cluster producer for the trigger pad downstream of target"""
        cluster = TrigScintClusterProducer( 'trigScintClustersDown' )
        cluster.input_collection = 'trigScintDigisDn'
        cluster.output_collection= 'TriggerPadDownClusters'
        return cluster

    def tagger() :
        """Get the cluster producer for the trigger pad upstream of tagger"""
        cluster = TrigScintClusterProducer( 'trigScintClustersTag' )
        cluster.input_collection = 'trigScintDigisTag'
        cluster.output_collection= 'TriggerPadTaggerClusters'
        return cluster

