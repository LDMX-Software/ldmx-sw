from LDMX.Framework import Processor, processor
from LDMX.Framework import ConditionsObjectProvider, conditions_object_provider


@processor("tracking::reco::RawTrackerDecoder", "Tracking")
class RawTrackerDecoder(Processor):
    """Decode raw Rogue frame bytes into a RawSiStripHit collection.

    Attributes
    ----------
    input_collection : str
        Raw byte buffer collection name from SingleSubsystemUnpacker.
    input_pass_name : str
        Pass name of the upstream producer (empty = any pass).
    output_collection : str
        Name for the output RawSiStripHit collection on the event bus.
    n_samples : int
        ADC samples per hit (always 3 for APV25).
    """

    input_collection: str = "TrackerRawData"
    input_pass_name: str = ""
    output_collection: str = "RawSiStripHits"
    n_samples: int = 3


@processor("tracking::reco::PedestalCalculator", "Tracking")
class PedestalCalculator(Processor):
    """Compute per-channel, per-sample pedestal mean and noise from a baseline run.

    Run this on a baseline/pedestal Rogue file decoded through the same
    SingleSubsystemUnpacker + RawTrackerDecoder pipeline.  On process end,
    writes a JSON file that PedestalSubtractor reads during physics reconstruction.

    Attributes
    ----------
    input_collection : str
        RawSiStripHit collection name to read (default: 'RawSiStripHits').
    input_pass_name : str
        Pass name of the upstream decoder (empty = any pass).
    output_file : str
        Path to write the pedestal file.
    output_format : str
        Storage backend for the pedestals ('json'; future: 'sqlite', ...).
    """

    input_collection: str = "RawSiStripHits"
    input_pass_name: str = ""
    output_file: str = "pedestals.json"
    output_format: str = "json"


@conditions_object_provider(
    "TrackerPedestals", "tracking::reco::TrackerPedestalProvider", "Tracking"
)
class TrackerPedestalProvider(ConditionsObjectProvider):
    """Serve per-channel tracker pedestals/noise as a conditions object.

    Reads the pedestal file written by PedestalCalculator and provides the
    'TrackerPedestals' conditions object, which PedestalSubtractor and
    SiStripWaveformBuilder query via getCondition<>().  The storage backend is
    selected here, so the choice of source is decoupled from the processors.

    Attributes
    ----------
    pedestal_file : str
        Path to the file produced by PedestalCalculator.
    pedestal_format : str
        Storage backend for the pedestals ('json'; future: 'sqlite', ...).
    """

    pedestal_file: str = "pedestals.json"
    pedestal_format: str = "json"


@processor("tracking::reco::PedestalSubtractor", "Tracking")
class PedestalSubtractor(Processor):
    """Subtract pedestals from a RawSiStripHit collection.

    Pulls the per-sample pedestal means from the TrackerPedestals conditions
    object (provided by TrackerPedestalProvider) and writes a new,
    pedestal-subtracted RawSiStripHit collection.  No file is read here and no
    noise is stored on the hits.

    Attributes
    ----------
    input_collection : str
        RawSiStripHit collection to subtract from (default: 'RawSiStripHits').
    input_pass_name : str
        Pass name of the upstream decoder (empty = any pass).
    output_collection : str
        Name for the subtracted RawSiStripHit collection (default: 'TrackerHits').
    """

    input_collection: str = "RawSiStripHits"
    input_pass_name: str = ""
    output_collection: str = "TrackerHits"


@processor("tracking::reco::SiStripWaveformBuilder", "Tracking")
class SiStripWaveformBuilder(Processor):
    """Assemble per-trigger subtracted hits into full per-channel waveforms.

    Groups all APV trigger hits for the same (feb, hybrid, pchannel), sorts
    by apv_trigger, and concatenates their samples into a SiStripWaveform
    with n_triggers*3 samples.  Per-channel noise for the significance cuts is
    taken from the TrackerPedestals conditions object.  A two-stage significance
    threshold is applied so only channels with real signal are written out.

    Attributes
    ----------
    input_collection : str
        Subtracted RawSiStripHit collection to read (default: 'TrackerHits').
    input_pass_name : str
        Pass name of the upstream producer (empty = any pass).
    output_collection : str
        Name for the output SiStripWaveform collection (default: 'TrackerWaveforms').
    high_threshold : float
        Per-sample significance (ADC/noise) for the high-threshold count cut.
    min_high_samples : int
        Minimum number of samples that must exceed high_threshold.
    low_threshold : float
        Per-sample significance (ADC/noise) for the consecutive-streak cut.
    min_consecutive_low : int
        Minimum number of consecutive samples that must exceed low_threshold.
    n_triggers : int
        Expected number of APV triggers per RoR (default: 10).
    verbose : bool
        Print full waveform traces to stdout (default: False).
    """

    input_collection: str = "TrackerHits"
    input_pass_name: str = ""
    output_collection: str = "TrackerWaveforms"
    high_threshold: float = 5.0
    min_high_samples: int = 4
    low_threshold: float = 3.0
    min_consecutive_low: int = 5
    n_triggers: int = 10
    verbose: bool = False
