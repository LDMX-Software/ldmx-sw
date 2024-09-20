"""Configuration for Trigger Scintillator digitization, cluster, and track producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.TrigScint.trigScint import TrigScintDigiProducer
    p.sequence.extend([ TrigScintDigiProducer.pad1() , TrigScintDigiProducer.pad2() , TrigScintDigiProducer.pad3() ])
    from LDMX.TrigScint.trigScint import TrigScintClusterProducer
    p.sequence.extend([ TrigScintClusterProducer.pad1() , TrigScintClusterProducer.pad2(), TrigScintClusterProducer.pad3() ]) 

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
        self.input_collection="TriggerPad3SimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintDigisPad3"
        import time
        self.randomSeed = int(time.time())
        self.verbose = False

    def pad1() :
        """Get the digitizer for the trigger pad most upstream of tagger"""
        digi = TrigScintDigiProducer( 'trigScintDigisPad1' )
        digi.input_collection = 'TriggerPad1SimHits'
        digi.output_collection= 'trigScintDigisPad1'
        return digi

    def pad2() :
        """Get the digitizer for the trigger pad just upstream of tagger"""
        digi = TrigScintDigiProducer( 'trigScintDigisPad2' )
        digi.input_collection = 'TriggerPad2SimHits'
        digi.output_collection= 'trigScintDigisPad2'
        return digi

    def pad3() :
        """Get the digitizer for the trigger pad upstream of target"""
        digi = TrigScintDigiProducer( 'trigScintDigisPad3' )
        digi.input_collection = 'TriggerPad3SimHits'
        digi.output_collection= 'trigScintDigisPad3'
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
        self.input_collection="TriggerPad3SimHits"
        self.input_pass_name="" #take any pass
        self.output_collection="trigScintQIEDigisPad3"
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

    def pad3() :
        """Get the digitizer for the trigger pad upstream of target"""
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisPad3' )
        digi.input_collection = 'TriggerPad3SimHits'
        digi.output_collection= 'trigScintQIEDigisPad3'
        return digi

    def pad1() :
        """Get the digitizer for the first trigger pad """
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisPad1' )
        digi.input_collection = 'TriggerPad1SimHits'
        digi.output_collection= 'trigScintQIEDigisPad1'
        return digi

    def pad2() :
        """Get the digitizer for the second trigger pad """
        digi = TrigScintQIEDigiProducer( 'trigScintQIEDigisPad2' )
        digi.input_collection = 'TriggerPad2SimHits'
        digi.output_collection= 'trigScintQIEDigisPad2'
        return digi


class EventReadoutProducer(ldmxcfg.Producer) :
    """Configuration for rechit producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::EventReadoutProducer','TrigScint')

        self.input_collection="decodedQIEPad1"
        self.input_pass_name=""   #take any pass
        self.output_collection="QIEsamplesPad1"
        self.number_pedestal_samples=5
        self.time_shift=5
        self.fiber_to_shift=0
        self.verbose = False
        
class TestBeamHitProducer(ldmxcfg.Producer) :
    """Configuration for testbeam hit producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TestBeamHitProducer','TrigScint')

        self.inputCollection="QIEsamplesPad1"
        self.inputPassName=""   #take any pass
        self.outputCollection="testBeamHitsPad1"
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
        self.input_collection="testBeamHitsPad1"
        self.input_pass_name="" #take any pass
        self.output_collection="TestBeamClustersPad1"
        self.doCleanHits = False   #whether to apply quality criteria from hit reconstruction
        self.verbosity = 0

    def pad1() :
        """Get the cluster producer for the trigger pad upstream of hcal """
        cluster = TestBeamClusterProducer( 'testBeamClustersPad1' )
        cluster.input_collection = 'testBeamHitsPad1'
        cluster.output_collection= 'TeastBeamClustersPad1'
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
        self.input_collection="trigScintQIEDigisPad3"
        self.input_pass_name=""   #take any pass
        self.output_collection="trigScintRecHitsPad3"
        self.verbose = False
        self.sample_of_interest=2 # Sample of interest. Range 0 to 3

    def pad1() : 
        """Get the rechit producer for first pad"""
        rechit = TrigScintRecHitProducer( 'trigScintRecHitsPad1' )
        rechit.input_collection  = 'trigScintQIEDigisPad1'
        rechit.output_collection = 'trigScintRecHitsPad1'
        return rechit

    def pad2() : 
        """Get the rechit producer for second pad"""
        rechit = TrigScintRecHitProducer( 'trigScintRecHitsPad2' )
        rechit.input_collection  = 'trigScintQIEDigisPad2'
        rechit.output_collection = 'trigScintRecHitsPad2'
        return rechit

    def pad3() : 
        """Get the rechit producer for third pad"""
        rechit = TrigScintRecHitProducer( 'trigScintRecHitsPad3' )
        rechit.input_collection  = 'trigScintQIEDigisPad3'
        rechit.output_collection = 'trigScintRecHitsPad3'
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
        self.vertical_bar_start_index = 52
        self.input_collection="trigScintDigisPad1"
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPad1Clusters"
        self.verbosity = 0

    def pad1() :
        """Get the cluster producer for the trigger pad most upstream of tagger"""
        cluster = TrigScintClusterProducer( 'trigScintClustersPad1' )
        cluster.input_collection = 'trigScintDigisPad1'
        cluster.output_collection= 'TriggerPad1Clusters'
        cluster.pad_time= -2.9
        return cluster

    def pad2() :
        """Get the cluster producer for the trigger pad just upstream of tagger"""
        cluster = TrigScintClusterProducer( 'trigScintClustersPad2' )
        cluster.input_collection = 'trigScintDigisPad2'
        cluster.output_collection= 'TriggerPad2Clusters'
        cluster.pad_time= -2.7
        return cluster

    def pad3() :
        """Get the cluster producer for the trigger pad just upstream of target"""
        cluster = TrigScintClusterProducer( 'trigScintClustersPad3' )
        cluster.input_collection = 'trigScintDigisPad3'
        cluster.output_collection= 'TriggerPad3Clusters'
        cluster.pad_time= 0.
        return cluster


class TrigScintTrackProducer(ldmxcfg.Producer) :
    """Configuration for track producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::TrigScintTrackProducer','TrigScint')

        self.delta_max = 0.75
        self.tracking_threshold = 0.  #to add in neighboring channels
        self.seeding_collection = "TriggerPad1Clusters"
        self.further_input_collections = ["TriggerPad2Clusters","TriggerPad3Clusters"]
        self.allow_skip_last_collection = False
        self.vertical_bar_start_index = 52
        self.number_horizontal_bars = 24  #16 for x,y segmented geometry only 
        self.number_vertical_bars = 0     #8 for x,y segmented geometry only
        self.horizontal_bar_width = 3.
        self.horizontal_bar_gap = 0.3
        self.vertical_bar_width = 3.
        self.vertical_bar_gap = 0.3
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTracks"
        self.verbosity = 0

trigScintTrack = TrigScintTrackProducer( "trigScintTrack" )

class TrigScintTrackProducer(ldmxcfg.Producer) :
    """Configuration for track producer for Trigger Scintillators"""

    def __init__(self,name) :
        super().__init__(name,'trigscint::','TrigScint')

        self.delta_max = 0.75
        self.tracking_threshold = 0.  #to add in neighboring channels
        self.seeding_collection = "TriggerPad1Clusters"
        self.further_input_collections = ["TriggerPad2Clusters","TriggerPad3Clusters"]
        self.allow_skip_last_collection = False
        self.vertical_bar_start_index = 52
        self.number_horizontal_bars = 24  #16 for x,y segmented geometry only 
        self.number_vertical_bars = 0     #8 for x,y segmented geometry only
        self.horizontal_bar_width = 3.
        self.horizontal_bar_gap = 0.3
        self.vertical_bar_width = 3.
        self.vertical_bar_gap = 0.3
        self.input_pass_name="" #take any pass
        self.output_collection="TriggerPadTracks"
        self.verbosity = 0

class TrigScintFirmwareTracker(ldmxcfg.Producer) :
    """Configuration for the track producer from the Firmware Tracker"""
    def __init__(self,name) :
        super().__init__(name,'trigscint::TrigScintFirmwareTracker','TrigScint')
        self.clustering_threshold=40.0
        self.digis1_collection='trigScintDigisPad1'
        self.digis2_collection='trigScintDigisPad2'
        self.digis3_collection='trigScintDigisPad3'
        self.input_pass_name=""
        self.output_collection="TriggerPadTracks"
        self.verbosity = 0
        self.time_tolerance = 50.0
        self.pad_time = -1.5

class QIEAnalyzer(ldmxcfg.Analyzer) :
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""
    
    def __init__(self,name) :
        super().__init__(name,'trigscint::QIEAnalyzer','TrigScint')

        self.inputCollection="QIEsamplesPad1"
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

        self.inputCollection="testBeamHitsPad1"
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
                 

class TestBeamClusterAnalyzer(ldmxcfg.Analyzer) :
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""
    
    def __init__(self,name) :
        super().__init__(name,'trigscint::TestBeamClusterAnalyzer','TrigScint')

        self.inputCollection="TestBeamClustersUpClean"
        self.inputPassName=""   #take any pass                                                                                         
        self.inputHitCollection="testBeamHitsUp"
        self.inputHitPassName=""   #take any pass                                                                                         
        self.startSample=2      #first time sample included in reformatting 
        self.deadChannels=[ 8 ]
                 
