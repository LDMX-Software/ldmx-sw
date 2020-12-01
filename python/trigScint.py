"""Configuration for Trigger Scintillator digitization, cluster, and track producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.TrigScint.trigScint import TrigScintDigiProducer
    p.sequence.extend([ TrigScintDigiProducer.up() , TrigScintDigiProducer.down() , TrigScintDigiProducer.tagger() ])
    from LDMX.TrigScint.trigScint import TrigScintClusterProducer
    p.sequence.extend([ TrigScintClusterProducer.up() , TrigScintClusterProducer.down() , TrigScintClusterProducer.tagger() ])

"""

from LDMX.Framework import ldmxcfg

class TrigScintDigiProducer(ldmxcfg.Producer) :
    """Configuration for digitizer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintDigiProducer','TrigScint')

        self.mean_noise = 0.02
        self.number_of_strips = 50
        self.number_of_arrays = 1
        self.mev_per_mip = 0.4
        self.pe_per_mip = 100.
        self.input_collection="TriggerPadUpSimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintDigisUp"
        import time
        self.randomSeed = int(time.time())
        self.verbose = False

    def up() :
        """Get the digitizer for the trigger pad upstream of target"""
        digi = TrigScintDigiProducer( 'trigScintDigisUp' )
        digi.input_collection = 'TriggerPadUpSimHits'
        digi.output_collection= 'trigScintDigisUp'
        return digi

    def down() :
        """Get the digitizer for the trigger pad downstream of target"""
        digi = TrigScintDigiProducer( 'trigScintDigisDn' )
        digi.input_collection = 'TriggerPadDownSimHits'
        digi.output_collection= 'trigScintDigisDn'
        return digi

    def tagger() :
        """Get the digitizer for the trigger pad upstream of tagger"""
        digi = TrigScintDigiProducer( 'trigScintDigisTag' )
        digi.input_collection = 'TriggerPadTaggerSimHits'
        digi.output_collection= 'trigScintDigisTag'
        return digi


class TrigScintQIEDigiProducer(ldmxcfg.Producer) :
    """Configuration for digitizer for Trigger Scintillators's QIE chip"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintQIEDigiProducer','TrigScint')

        self.mean_noise = 0.02
        self.number_of_strips = 50
        self.number_of_arrays = 1
        self.mev_per_mip = 0.4
        self.pe_per_mip = 100.
        self.input_collection="TriggerPadUpSimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintQIEDigisUp"
        self.input_pulse_shape="Expo" # Name of the input pulse class
        self.expo_k=0.1
        self.expo_tmax=5.0
        self.maxts=5
        self.toff_overall = 30.0
        import time
        self.randomSeed = int(time.time())
        self.verbose = False

    def up() :
        """Get the digitizer for the trigger pad upstream of target"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisUp' )
        digi.input_collection = 'TriggerPadUpSimHits'
        digi.output_collection= 'trigScintQIEDigisUp'
        return digi

    def down() :
        """Get the digitizer for the trigger pad downstream of target"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisDn' )
        digi.input_collection = 'TriggerPadDownSimHits'
        digi.output_collection= 'trigScintQIEDigisDn'
        return digi

    def tagger() :
        """Get the digitizer for the trigger pad upstream of tagger"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisTag' )
        digi.input_collection = 'TriggerPadTaggerSimHits'
        digi.output_collection= 'trigScintQIEDigisTag'
        return digi


class TrigScintClusterProducer(ldmxcfg.Producer) :
    """Configuration for cluster producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintClusterProducer','TrigScint')

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


from LDMX.Framework import ldmxcfg

class TrigScintTrackProducer(ldmxcfg.Producer) :
    """Configuration for track producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'ldmx::TrigScintTrackProducer','TrigScint')

        self.delta_max = 1.0
        self.tracking_threshold = 0.  #to add in neighboring channels
        self.seeding_collection = "TriggerPadTaggerClusters"
        self.further_input_collections = ["TriggerPadUpClusters","TriggerPadDownClusters"]
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTracks"
        self.verbosity = 0



trigScintTrack = TrigScintTrackProducer( "trigScintTrack" )

