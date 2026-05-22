"""Configuration for Trigger Scintillator digitization, cluster, and track producers

Sets all parameters to reasonable defaults.

Examples
--------
    from LDMX.TrigScint.trigScint import TrigScintDigiProducer
    p.sequence.extend([
        TrigScintDigiProducer.pad1(),
        TrigScintDigiProducer.pad2(),
        TrigScintDigiProducer.pad3(),
    ])
    from LDMX.TrigScint.trigScint import TrigScintClusterProducer
    p.sequence.extend([
        TrigScintClusterProducer.pad1(),
        TrigScintClusterProducer.pad2(),
        TrigScintClusterProducer.pad3(),
    ])

"""

from LDMX.Framework import Processor, processor


@processor("trigscint::TrigScintDigiProducer", "TrigScint")
class TrigScintDigiProducer(Processor):
    """Configuration for digitizer for Trigger Scintillators"""

    mean_noise: float = 0.02
    number_of_strips: int = 50
    number_of_arrays: int = 1
    mev_per_mip: float = 0.4
    pe_per_mip: float = 100.0
    input_collection: str = "TriggerPad3SimHits"
    input_pass_name: str = ""
    output_collection: str = "trigScintDigisPad3"
    sim_particles_coll_name: str = "SimParticles"
    sim_particles_passname: str = ""

    @staticmethod
    def pad1(**kwargs):
        """Get the digitizer for the trigger pad most upstream of tagger"""
        return TrigScintDigiProducer(
            instance_name="trigScintDigisPad1",
            input_collection="TriggerPad1SimHits",
            output_collection="trigScintDigisPad1",
            **kwargs,
        )

    @staticmethod
    def pad2(**kwargs):
        """Get the digitizer for the trigger pad just upstream of tagger"""
        return TrigScintDigiProducer(
            instance_name="trigScintDigisPad2",
            input_collection="TriggerPad2SimHits",
            output_collection="trigScintDigisPad2",
            **kwargs,
        )

    @staticmethod
    def pad3(**kwargs):
        """Get the digitizer for the trigger pad upstream of target"""
        return TrigScintDigiProducer(
            instance_name="trigScintDigisPad3",
            input_collection="TriggerPad3SimHits",
            output_collection="trigScintDigisPad3",
            **kwargs,
        )

    @staticmethod
    def target(**kwargs):
        """Get the digitizer for the active target"""
        return TrigScintDigiProducer(
            instance_name="TargetDigis",
            input_collection="TargetSimHits",
            output_collection="TargetDigis",
            mev_per_mip=1.0,
            pe_per_mip=300.0,
            **kwargs,
        )


@processor("trigscint::TrigScintQIEDigiProducer", "TrigScint")
class TrigScintQIEDigiProducer(Processor):
    """Configuration for digitizer for Trigger Scintillators's QIE chip"""

    mean_noise: float = 0.02
    number_of_strips: int = 50
    number_of_arrays: int = 1
    mev_per_mip: float = 0.4
    pe_per_mip: float = 100.0
    input_collection: str = "TriggerPad3SimHits"
    input_pass_name: str = ""
    output_collection: str = "trigScintQIEDigisPad3"
    input_pulse_shape: str = "Expo"
    expo_k: float = 0.1
    expo_tmax: float = 5.0
    maxts: int = 5
    toff_overall: float = 55.0
    tdc_thr: float = 3.4
    pedestal: float = 6.0
    elec_noise: float = 1.5
    sipm_gain: float = 1.0e6
    qie_sf: float = 40.0
    zero_supp_in_pe: float = 1.0
    verbose: bool = False

    @staticmethod
    def pad3(**kwargs):
        """Get the digitizer for the trigger pad upstream of target"""
        return TrigScintQIEDigiProducer(
            instance_name="trigScintQIEDigisPad3",
            input_collection="TriggerPad3SimHits",
            output_collection="trigScintQIEDigisPad3",
            **kwargs,
        )

    @staticmethod
    def pad1(**kwargs):
        """Get the digitizer for the first trigger pad"""
        return TrigScintQIEDigiProducer(
            instance_name="trigScintQIEDigisPad1",
            input_collection="TriggerPad1SimHits",
            output_collection="trigScintQIEDigisPad1",
            **kwargs,
        )

    @staticmethod
    def pad2(**kwargs):
        """Get the digitizer for the second trigger pad"""
        return TrigScintQIEDigiProducer(
            instance_name="trigScintQIEDigisPad2",
            input_collection="TriggerPad2SimHits",
            output_collection="trigScintQIEDigisPad2",
            **kwargs,
        )


@processor("trigscint::EventReadoutProducer", "TrigScint")
class EventReadoutProducer(Processor):
    """Configuration for rechit producer for Trigger Scintillators"""

    input_collection: str = "decodedQIEPad1"
    input_pass_name: str = ""
    output_collection: str = "QIEsamplesPad1"
    number_pedestal_samples: int = 5
    time_shift: int = 5
    fiber_to_shift: int = 0
    verbose: bool = False


@processor("trigscint::TestBeamHitProducer", "TrigScint")
class TestBeamHitProducer(Processor):
    """Configuration for testbeam hit producer for Trigger Scintillators"""

    input_collection: str = "QIEsamplesPad1"
    input_pass_name: str = ""
    output_collection: str = "testBeamHitsPad1"
    verbose: bool = False
    do_clean_hits: bool = False
    n_instrumented_channels: int = 12
    start_sample: int = 10
    pulse_width: int = 5
    pulse_width_lyso: int = 8
    gain: list[float] = [2.0e6] * 12
    MIPresponse: list[float] = [1.0] * 12
    pedestals: list[float] = [
        -4.6,
        -2.6,
        -0.6,
        4.5,
        1.9,
        -2.2,
        0.9,
        -1.2,
        4.8,
        -4.4,
        -0.1,
        -1.7,
        3.3,
        -0.3,
        1.3,
        1.3,
    ]


@processor("trigscint::TestBeamClusterProducer", "TrigScint")
class TestBeamClusterProducer(Processor):
    """Configuration for cluster producer for Trigger Scintillators"""

    max_cluster_width: int = 2
    max_channel_nb: int = 11
    clustering_threshold: float = 40.0
    seed_threshold: float = 60.0
    pad_time: float = -999.0
    time_tolerance: float = 50.0
    input_collection: str = "testBeamHitsPad1"
    input_pass_name: str = ""
    output_collection: str = "TestBeamClustersPad1"
    do_clean_hits: bool = False
    verbosity: int = 0

    @staticmethod
    def pad1(**kwargs):
        """Get the cluster producer for the trigger pad upstream of hcal"""
        return TestBeamClusterProducer(
            instance_name="testBeamClustersPad1",
            input_collection="testBeamHitsPad1",
            output_collection="TeastBeamClustersPad1",
            pad_time=-999.0,
            **kwargs,
        )


@processor("trigscint::TrigScintRecHitProducer", "TrigScint")
class TrigScintRecHitProducer(Processor):
    """Configuration for rechit producer for Trigger Scintillators"""

    mev_per_mip: float = 0.4
    pe_per_mip: float = 100.0
    pedestal: float = 6.0
    gain: float = 1.0e6
    input_collection: str = "trigScintQIEDigisPad3"
    input_pass_name: str = ""
    output_collection: str = "trigScintRecHitsPad3"
    verbose: bool = False
    sample_of_interest: int = 2

    @staticmethod
    def pad1(**kwargs):
        """Get the rechit producer for first pad"""
        return TrigScintRecHitProducer(
            instance_name="trigScintRecHitsPad1",
            input_collection="trigScintQIEDigisPad1",
            output_collection="trigScintRecHitsPad1",
            **kwargs,
        )

    @staticmethod
    def pad2(**kwargs):
        """Get the rechit producer for second pad"""
        return TrigScintRecHitProducer(
            instance_name="trigScintRecHitsPad2",
            input_collection="trigScintQIEDigisPad2",
            output_collection="trigScintRecHitsPad2",
            **kwargs,
        )

    @staticmethod
    def pad3(**kwargs):
        """Get the rechit producer for third pad"""
        return TrigScintRecHitProducer(
            instance_name="trigScintRecHitsPad3",
            input_collection="trigScintQIEDigisPad3",
            output_collection="trigScintRecHitsPad3",
            **kwargs,
        )


@processor("trigscint::TrigScintFirmwareHitProducer", "TrigScint")
class TrigScintFirmwareHitProducer(Processor):
    """Configuration for rechit producer for Trigger Scintillators
    incorporating validated Firmware, regular and pileUp"""

    mev_per_mip: float = 0.4
    pe_per_mip: float = 100.0
    pedestal: float = 6.0
    gain: float = 1.0e6
    input_collection: str = "trigScintQIEDigisPad3"
    test_collection: str = "trigScintRecHitsPad3"
    input_pass_name: str = ""
    output_collection: str = "trigScintFirmHitsPad3"
    verbose: bool = False
    sample_of_interest: int = 2

    @staticmethod
    def pad1(**kwargs):
        """Get the firmware hit producer for first pad"""
        return TrigScintRecHitProducer(
            instance_name="trigScintFirmHitsPad1",
            input_collection="trigScintQIEDigisPad1",
            output_collection="trigScintFirmHitsPad1",
            test_collection="trigScintRecHitsPad1",
            **kwargs,
        )

    @staticmethod
    def pad2(**kwargs):
        """Get the firmware hit producer for second pad"""
        return TrigScintRecHitProducer(
            instance_name="trigScintFirmHitsPad2",
            input_collection="trigScintQIEDigisPad2",
            output_collection="trigScintFirmHitsPad2",
            test_collection="trigScintRecHitsPad2",
            **kwargs,
        )

    @staticmethod
    def pad3(**kwargs):
        """Get the firmware hit for third pad"""
        return TrigScintRecHitProducer(
            instance_name="trigScintFirmHitsPad3",
            input_collection="trigScintQIEDigisPad3",
            output_collection="trigScintFirmHitsPad3",
            test_collection="trigScintRecHitsPad3",
            **kwargs,
        )


@processor("trigscint::TrigScintClusterProducer", "TrigScint")
class TrigScintClusterProducer(Processor):
    """Configuration for cluster producer for Trigger Scintillators"""

    max_cluster_width: int = 2
    ampl_weighting: bool = True
    clustering_threshold: float = 0.0
    seed_threshold: float = 30.0
    pad_time: float = 0.0
    time_tolerance: float = 0.5
    vertical_bar_start_index: int = 52
    input_collection: str = "trigScintDigisPad1"
    input_pass_name: str = ""
    output_collection: str = "TriggerPad1Clusters"
    verbosity: int = 0

    @staticmethod
    def pad1(**kwargs):
        """Get the cluster producer for the trigger pad most upstream of tagger"""
        return TrigScintClusterProducer(
            instance_name="trigScintClustersPad1",
            input_collection="trigScintDigisPad1",
            output_collection="TriggerPad1Clusters",
            pad_time=-2.9,
            **kwargs,
        )

    @staticmethod
    def pad2(**kwargs):
        """Get the cluster producer for the trigger pad just upstream of tagger"""
        return TrigScintClusterProducer(
            instance_name="trigScintClustersPad2",
            input_collection="trigScintDigisPad2",
            output_collection="TriggerPad2Clusters",
            pad_time=-2.7,
            **kwargs,
        )

    @staticmethod
    def pad3(**kwargs):
        """Get the cluster producer for the trigger pad just upstream of target"""
        return TrigScintClusterProducer(
            instance_name="trigScintClustersPad3",
            input_collection="trigScintDigisPad3",
            output_collection="TriggerPad3Clusters",
            pad_time=0.0,
            **kwargs,
        )

    @staticmethod
    def target(**kwargs):
        """Get the cluster producer for the active target"""
        return TrigScintClusterProducer(
            instance_name="TargetClusters",
            input_collection="TargetDigis",
            output_collection="TargetClusters",
            pad_time=0.0,
            **kwargs,
        )


@processor("trigscint::TrigScintTrackProducer", "TrigScint")
class TrigScintTrackProducer(Processor):
    """Configuration for track producer for Trigger Scintillators"""

    delta_max: float = 0.75
    tracking_threshold: float = 0.0
    seeding_collection: str = "TriggerPad1Clusters"
    further_input_collections: list[str] = [
        "TriggerPad2Clusters",
        "TriggerPad3Clusters",
    ]
    allow_skip_last_collection: bool = False
    vertical_bar_start_index: int = 52
    number_horizontal_bars: int = 24
    number_vertical_bars: int = 0
    horizontal_bar_width: float = 3.0
    horizontal_bar_gap: float = 0.3
    vertical_bar_width: float = 3.0
    vertical_bar_gap: float = 0.3
    input_pass_name: str = ""
    output_collection: str = "TriggerPadTracks"
    verbosity: int = 0


trig_scint_track = TrigScintTrackProducer(instance_name="trig_scint_track")


@processor("trigscint::TrigScintFirmwareTracker", "TrigScint")
class TrigScintFirmwareTracker(Processor):
    """Configuration for the track producer from the Firmware Tracker"""

    clustering_threshold: float = 40.0
    digis1_collection: str = "trigScintDigisPad1"
    digis2_collection: str = "trigScintDigisPad2"
    digis3_collection: str = "trigScintDigisPad3"
    input_pass_name: str = ""
    output_collection: str = "TriggerPadTracks"
    verbosity: int = 0
    time_tolerance: float = 50.0
    pad_time: float = -1.5


@processor("trigscint::QIEAnalyzer", "TrigScint")
class QIEAnalyzer(Processor):
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""

    input_collection: str = "QIEsamplesPad1"
    input_pass_name: str = ""
    start_sample: int = 2
    gain: list[float] = [2.0e6] * 16
    pedestals: list[float] = [
        -4.6,
        -2.6,
        -0.6,
        4.5,
        1.9,
        -2.2,
        0.9,
        -1.2,
        4.8,
        -4.4,
        -0.1,
        -1.7,
        3.3,
        -0.3,
        1.3,
        1.3,
    ]


@processor("trigscint::QualityFlagAnalyzer", "TrigScint")
class QualityFlagAnalyzer(Processor):
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""

    input_event_collection: str = "QIEsamplesUp"
    input_event_pass_name: str = ""
    input_hit_collection: str = "testBeamHitsUp"
    input_hit_pass_name: str = ""
    start_sample: int = 2
    gain: list[float] = [2.0e6] * 16
    pedestals: list[float] = [
        -4.6,
        -2.6,
        -0.6,
        4.5,
        1.9,
        -2.2,
        0.9,
        -1.2,
        4.8,
        -4.4,
        -0.1,
        -1.7,
        3.3,
        -0.3,
        1.3,
        1.3,
    ]


@processor("trigscint::TestBeamHitAnalyzer", "TrigScint")
class TestBeamHitAnalyzer(Processor):
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""

    input_collection: str = "testBeamHitsPad1"
    input_pass_name: str = ""
    start_sample: int = 2
    pedestals: list[float] = [
        -4.6,
        -2.6,
        -0.6,
        4.5,
        1.9,
        -2.2,
        0.9,
        -1.2,
        4.8,
        -4.4,
        -0.1,
        -1.7,
        3.3,
        -0.3,
        1.3,
        1.3,
    ]


@processor("trigscint::TestBeamClusterAnalyzer", "TrigScint")
class TestBeamClusterAnalyzer(Processor):
    """Configuration for linearized QIE analyzer for Trigger Scintillators"""

    input_collection: str = "TestBeamClustersUpClean"
    input_pass_name: str = ""
    input_hit_collection: str = "testBeamHitsUp"
    input_hit_pass_name: str = ""
    start_sample: int = 2
    dead_channels: list[int] = [8]
