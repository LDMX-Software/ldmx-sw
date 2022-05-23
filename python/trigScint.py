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
        super().__init__(name,'trigscint::TrigScintDigiProducer','TrigScint')

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
        super().__init__(name,'trigscint::TrigScintQIEDigiProducer','TrigScint')

        self.mean_noise = 0.02
        self.number_of_strips = 50
        self.number_of_arrays = 1
        self.mev_per_mip = 0.4
        self.pe_per_mip = 100.
        self.input_collection="TriggerPadUpSimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintQIEDigisUp"
        self.input_pulse_shape="Expo" # Name of the input pulse class
        self.expo_k=0.1          # Inverse of decay time of piece-wise exponential 
        self.expo_tmax=5.0       # Time at which piece-wise exponential peaks
        self.maxts=5             # No. of time samples to analyze
        self.toff_overall = 55.0 # Global time offset
        self.tdc_thr = 3.4       # Threshold current in uA for TDC latch
        self.pedestal= 6.0       # QIE pedestal value (in fC)
        self.elec_noise = 1.5    # Electronic noise (in fC)
        self.sipm_gain = 1.e6    # SiPM Gain
        self.qie_sf = 40.        # QIE sampling frequency in MHz
        self.zeroSupp_in_pe = 1. # min nPE in integrated pulse to keep hit  
        
        import time
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


class EventReadoutProducer(ldmxcfg.Producer) :
    """Configuration for rechit producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::EventReadoutProducer','TrigScint')

        self.input_collection="decodedQIEUp"
        self.input_pass_name=""   #take any pass
        self.output_collection="QIEsamplesUp"
        self.number_pedestal_samples=5
        self.time_shift=5
        self.fiber_to_shift=0
        self.verbose = False
        
class TestBeamHitProducer(ldmxcfg.Producer) :
    """Configuration for testbeam hit producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TestBeamHitProducer','TrigScint')

        self.inputCollection="QIEsamplesUp"
        self.inputPassName=""   #take any pass
        self.outputCollection="testBeamHitsUp"
        self.verbose = False
        self.doCleanHits = False   #whether to apply quality criteria in hit reconstruction
        self.nInstrumentedChannels=12 #number of channels 
        self.startSample=10   # Sample where pulse is expected to start (triggered mode)
        self.pulseWidth=5     # Number of consecutive samples to include in the pulse
        self.pulseWidthLYSO=8 # as above, for LYSO 
        self.gain = [2.e6]*12      # SiPM Gain
        self.MIPresponse = [1.]*12      # channel MIP response correction factor 
        self.pedestals=[
            -4.6, #0.6,
            -2.6, #4.4,
            -0.6, #-1.25,
            4.5,  #3.9, 	 # #3
            1.9,  #10000., # #4: (used to be) dead channel during test beam
            -2.2, #-2.1,   # #5 
            0.9,  #2.9,    # #6
            -1.2, #-2,     # #7
            4.8,  #-0.4,   # #8
            -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
            -0.1, #1.5,    # #10
            -1.7, #2.0,    # #11
            3.3,  #3.7,    # #12 -- uninstrumented
            -0.3, #2.8,    # #13 -- uninstrumented
            1.3,  #-1.5,   # #14 -- uninstrumented
            1.3   #1.6     # #15 -- uninstrumented
        ]

                                
class TestBeamClusterProducer(ldmxcfg.Producer) :
    """Configuration for cluster producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TestBeamClusterProducer','TrigScint')

        self.max_cluster_width = 2
        self.max_channel_nb = 11
        self.clustering_threshold = 40.  #to add in neighboring channels
        self.seed_threshold = 60.
        self.pad_time = -999.
        self.time_tolerance = 50.
        self.input_collection="testBeamHitsUp"
        self.input_pass_name="" #take any pass
        self.output_collection="TestBeamClustersUp"
        self.doCleanHits = False   #whether to apply quality criteria from hit reconstruction
        self.verbosity = 0

    def up() :
        """Get the cluster producer for the trigger pad upstream of target"""
        cluster = TestBeamClusterProducer( 'testBeamClustersUp' )
        cluster.input_collection = 'testBeamHitsUp'
        cluster.output_collection= 'TeastBeamClustersUp'
        cluster.pad_time= -999.
        return cluster
        
class TrigScintRecHitProducer(ldmxcfg.Producer) :
    """Configuration for rechit producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TrigScintRecHitProducer','TrigScint')

        self .mev_per_mip = 0.4   #\
                                  # >>>both are for converting edep to PEs 
        self.pe_per_mip = 100.    #/
        self.pedestal= 6.0        # QIE pedestal value (in fC)
        self.gain = 1.e6      # SiPM Gain
        self.input_collection="trigScintQIEDigisUp"
        self.input_pass_name=""   #take any pass
        self.output_collection="trigScintRecHitsUp"
        self.verbose = False
        self.sample_of_interest=2 # Sample of interest. Range 0 to 3

    def up() : 
        """Get the rechit producer for upstream pad"""
        rechit = TrigScintRecHitProducer( 'trigScintRecHitsUp' )
        rechit.input_collection  = 'trigScintQIEDigisUp'
        rechit.output_collection = 'trigScintRecHitsUp'
        return rechit

    def down() : 
        """Get the rechit producer for downstream pad"""
        rechit = TrigScintRecHitProducer( 'trigScintRecHitsDown' )
        rechit.input_collection  = 'trigScintQIEDigisDn'
        rechit.output_collection = 'trigScintRecHitsDn'
        return rechit

    def tagger() : 
        """Get the rechit producer for tagger pad"""
        rechit = TrigScintRecHitProducer( 'trigScintRecHitsTag' )
        rechit.input_collection  = 'trigScintQIEDigisTag'
        rechit.output_collection = 'trigScintRecHitsTag'
        return rechit

class TrigScintClusterProducer(ldmxcfg.Producer) :
    """Configuration for cluster producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TrigScintClusterProducer','TrigScint')

        self.max_cluster_width = 2
        self.clustering_threshold = 0.  #to add in neighboring channels
        self.seed_threshold = 30.
        self.pad_time = 0.
        self.time_tolerance = 0.5
        self.input_collection="trigScintDigisTag"
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTaggerClusters"
        self.verbosity = 0

    def up() :
        """Get the cluster producer for the trigger pad upstream of target"""
        cluster = TrigScintClusterProducer( 'trigScintClustersUp' )
        cluster.input_collection = 'trigScintDigisUp'
        cluster.output_collection= 'TriggerPadUpClusters'
        cluster.pad_time= 0.
        return cluster

    def down() :
        """Get the cluster producer for the trigger pad downstream of target"""
        cluster = TrigScintClusterProducer( 'trigScintClustersDown' )
        cluster.input_collection = 'trigScintDigisDn'
        cluster.output_collection= 'TriggerPadDownClusters'
        cluster.pad_time= 0.
        return cluster

    def tagger() :
        """Get the cluster producer for the trigger pad upstream of tagger"""
        cluster = TrigScintClusterProducer( 'trigScintClustersTag' )
        cluster.input_collection = 'trigScintDigisTag'
        cluster.output_collection= 'TriggerPadTaggerClusters'
        cluster.pad_time= -2.
        return cluster


class TrigScintTrackProducer(ldmxcfg.Producer) :
    """Configuration for track producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TrigScintTrackProducer','TrigScint')

        self.delta_max = 0.75
        self.tracking_threshold = 0.  #to add in neighboring channels
        self.seeding_collection = "TriggerPadTaggerClusters"
        self.further_input_collections = ["TriggerPadUpClusters","TriggerPadDownClusters"]
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTracks"
        self.verbosity = 0

trigScintTrack = TrigScintTrackProducer( "trigScintTrack" )

class QIEAnalyzer(ldmxcfg.Analyzer) :
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""
    
    def __init__(self,name) :
        super().__init__(name,'trigscint::QIEAnalyzer','TrigScint')

        self.inputCollection="QIEsamplesUp"
        self.inputPassName=""   #take any pass                                                                                         
        self.startSample=2      #first time sample included in reformatting 
        self.gain = [2.e6]*16      # SiPM Gain  //TODO: vector 
        self.pedestals=[
            -4.6, #0.6,
            -2.6, #4.4,
            -0.6, #-1.25,
            4.5,  #3.9, 	 # #3
            1.9,  #10000., # #4: (used to be) dead channel during test beam
            -2.2, #-2.1,   # #5 
            0.9,  #2.9,    # #6
            -1.2, #-2,     # #7
            4.8,  #-0.4,   # #8
            -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
            -0.1, #1.5,    # #10
            -1.7, #2.0,    # #11
            3.3,  #3.7,    # #12 -- uninstrumented
            -0.3, #2.8,    # #13 -- uninstrumented
            1.3,  #-1.5,   # #14 -- uninstrumented
            1.3   #1.6     # #15 -- uninstrumented
        ]

        
class QualityFlagAnalyzer(ldmxcfg.Analyzer) :
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""
    
    def __init__(self,name) :
        super().__init__(name,'trigscint::QualityFlagAnalyzer','TrigScint')

        self.inputEventCollection="QIEsamplesUp"
        self.inputEventPassName=""   #take any pass                                                                                         
        self.inputHitCollection="testBeamHitsUp"
        self.inputHitPassName=""   #take any pass                                                                                         
        self.startSample=2      #first time sample included in reformatting 
        self.gain = [2.e6]*16      # SiPM Gain  //TODO: vector 
        self.pedestals=[
            -4.6, #0.6,
            -2.6, #4.4,
            -0.6, #-1.25,
            4.5,  #3.9, 	 # #3
            1.9,  #10000., # #4: (used to be) dead channel during test beam
            -2.2, #-2.1,   # #5 
            0.9,  #2.9,    # #6
            -1.2, #-2,     # #7
            4.8,  #-0.4,   # #8
            -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
            -0.1, #1.5,    # #10
            -1.7, #2.0,    # #11
            3.3,  #3.7,    # #12 -- uninstrumented
            -0.3, #2.8,    # #13 -- uninstrumented
            1.3,  #-1.5,   # #14 -- uninstrumented
            1.3   #1.6     # #15 -- uninstrumented
        ]

class TestBeamHitAnalyzer(ldmxcfg.Analyzer) :
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""
    
    def __init__(self,name) :
        super().__init__(name,'trigscint::TestBeamHitAnalyzer','TrigScint')

        self.inputCollection="testBeamHitsUp"
        self.inputPassName=""   #take any pass                                                                                         
        self.startSample=2      #first time sample included in reformatting 
        self.pedestals=[
            -4.6, #0.6,
            -2.6, #4.4,
            -0.6, #-1.25,
            4.5,  #3.9, 	 # #3
            1.9,  #10000., # #4: (used to be) dead channel during test beam
            -2.2, #-2.1,   # #5 
            0.9,  #2.9,    # #6
            -1.2, #-2,     # #7
            4.8,  #-0.4,   # #8
            -4.4, #-1.1,   # #9: dead channel in TTU teststand setup
            -0.1, #1.5,    # #10
            -1.7, #2.0,    # #11
            3.3,  #3.7,    # #12 -- uninstrumented
            -0.3, #2.8,    # #13 -- uninstrumented
            1.3,  #-1.5,   # #14 -- uninstrumented
            1.3   #1.6     # #15 -- uninstrumented
        ]
                 
