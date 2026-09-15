"""Config for the trigger-scintillator -> ldmx::Measurement producer.

Reads the decoded ZCCM per-pad digi collections and emits geometry-aware
ldmx::Measurement objects (global y from the bar geometry), in the same frame /
class the tracker uses. Geometry is parameterized here for this first increment
and tuned against the tracker beam; the follow-up promotes it to a DetDescr
ConditionsObject provider.
"""

from LDMX.Framework import Processor, processor


@processor("tracking::reco::TrigScintMeasurementProducer", "Tracking")
class TrigScintMeasurementProducer(Processor):
    """TS bar digis -> geometry-aware ldmx::Measurement.

    Attributes
    ----------
    input_collections : list[str]
        Decoded digi collection per module (index = module number). 3 nominal
        plastic pads; the LYSO pad (Pad4) is deferred.
    input_pass : str
        Pass name of the input collections ("" = any).
    out_collection : str
        Output ldmx::Measurement collection name.
    daq_map_file : str
        Optional TS DAQ map JSON (decode collection -> geometry module).
    amp_threshold : float
        ADC amplitude (max-min over samples) hit threshold.
    sigma_y : float
        Assumed y measurement resolution [mm].
    """

    input_collections: list[str] = ["decodedZCCMPad1", "decodedZCCMPad2", "decodedZCCMPad3"]
    input_pass: str = ""
    out_collection: str = "TrigScintMeasurements"

    # Optional TS DAQ map JSON (decode collection -> geometry module). When set,
    # it drives which collections are processed and their geometry module (see
    # LDMX.TrigScint.ts_daq_map.ts_daq_map_path). When empty, input_collections
    # is used with index = geometry module.
    daq_map_file: str = ""

    # Bar positions (module z, y pitch/stagger, ...) come from the
    # TrigScintGeometry conditions object -- import LDMX.TrigScint.trigscint_geometry
    # in the job so the provider is registered.
    amp_threshold: float = 40.0
    sigma_y: float = 0.9
