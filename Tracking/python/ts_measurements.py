"""Config for the trigger-scintillator -> ldmx::Measurement producer.

Reads reconstructed TS bar *clusters* (ldmx::TrigScintCluster, the output of the
standard TrigScintRecHitProducer -> TrigScintClusterProducer chain) and emits
geometry-aware ldmx::Measurement objects (global y from the bar geometry, at the
cluster's sub-bar centroid), in the same frame / class the tracker uses. Geometry
is parameterized here for this first increment and tuned against the tracker beam;
the follow-up promotes it to a DetDescr ConditionsObject provider.

The helper `ts_cluster_chain` builds the reco chain so drivers share one
definition of the calibration. It uses the standard rechit/cluster producers
(TrigScintRecHitProducer, TrigScintClusterProducer) -- NOT the TestBeamHit/
TestBeamCluster producers used by .github/validation_samples/esa_vst/config.py,
which are leftovers from an old CERN test-beam analysis.
"""

from LDMX.Framework import Processor, processor


@processor("tracking::reco::TrigScintMeasurementProducer", "Tracking")
class TrigScintMeasurementProducer(Processor):
    """TS bar clusters -> geometry-aware ldmx::Measurement.

    Attributes
    ----------
    input_collections : list[str]
        TrigScintCluster collection per module (index = geometry module). 3
        nominal plastic pads; the LYSO pad (Pad4) is deferred.
    input_pass : str
        Pass name of the input collections ("" = any).
    out_collection : str
        Output ldmx::Measurement collection name.
    daq_map_file : str
        Optional TS DAQ map JSON (cluster collection -> geometry module).
    min_pe : float
        Optional cluster PE floor (0 keeps every cluster; the clustering
        seed/threshold cuts already define the clusters).
    sigma_y : float
        Assumed y measurement resolution [mm].
    """

    input_collections: list[str] = [
        "TrigScintClustersPad1",
        "TrigScintClustersPad2",
        "TrigScintClustersPad3",
    ]
    input_pass: str = ""
    out_collection: str = "TrigScintMeasurements"

    # Optional TS DAQ map JSON (cluster collection -> geometry module). When set,
    # it drives which collections are processed and their geometry module (see
    # LDMX.TrigScint.ts_daq_map.ts_daq_map_path). When empty, input_collections
    # is used with index = geometry module.
    daq_map_file: str = ""

    # Bar positions (module z, y pitch/stagger, ...) come from the
    # TrigScintGeometry conditions object -- import LDMX.TrigScint.trigscint_geometry
    # in the job so the provider is registered.
    min_pe: float = 0.0
    sigma_y: float = 0.9


# Per-pad rechit sample-of-interest (pulse peak sample). Pad1's pulse peaks
# earlier than pads 2/3; these are the validated ESA25 hardcodes recommended by
# the TS group (per-bar sub-sample shift refinements are not yet merged and are
# intentionally not used here).
ESA25_SAMPLE_OF_INTEREST = {1: 5, 2: 8, 3: 8}


def ts_cluster_chain(
    pads=(1, 2, 3),
    input_pass="",
    sample_of_interest=None,
    integration_window=5,
    pedestal=-2.0,
    gain=2.0e6,
    mev_per_mip=0.4,
    pe_per_mip=100.0,
    seed_threshold=25.0,
    clustering_threshold=25.0,
    max_cluster_width=2,
    time_tolerance=10000.0,
    pad_time=0.0,
    calib_files=None,
):
    """Build the standard ZCCM-digi -> TrigScintCluster chain, per pad.

    Per pad p:
        decodedZCCMPad{p}  --TrigScintRecHitProducer-->  trigScintRecHitsPad{p}
                           --TrigScintClusterProducer--> TrigScintClustersPad{p}

    These are the STANDARD trigger-scintillator producers, matching the ESA25 TS
    reco config used by the TS group. The CI validation sample (esa_vst/config.py)
    instead runs EventReadoutProducer -> TestBeamHitProducer ->
    TestBeamClusterProducer, which are leftovers from an old CERN test-beam
    analysis; this chain replaces them. TrigScintRecHit reads the decoded QIE
    digis directly (no EventReadout stage) and both producers use the standard
    ldmx::TrigScintHit / ldmx::TrigScintCluster classes.

    Calibration
    -----------
    By default each pad's rechit reads per-channel gains/pedestals from the
    vendored ESA25 calibration file (TrigScint/data/esa25/calibration_pad{p}.txt,
    `channel gain pedestal` rows). This -- together with the per-pad
    `sample_of_interest` -- is what puts the three pads on a common PE scale
    (with a single scalar gain and a uniform sample, pad1 mis-integrates its
    earlier pulse and reads ~5x low). Pass `calib_files=False` to fall back to
    the scalar `pedestal`/`gain`, or `calib_files={pad: path}` for explicit files.

    Parameters
    ----------
    pads : iterable[int]
        Decode pad numbers to reconstruct (1-based; == decodedZCCMPad{p}).
    sample_of_interest : int | dict[int,int] | None
        Rechit pulse-peak sample. None uses the per-pad ESA25 defaults
        {1:5, 2:8, 3:8}; an int applies to all pads; a dict overrides per pad.
    integration_window : int
        Samples integrated starting at sample_of_interest (<=0 = to the end).
    seed_threshold, clustering_threshold : float
        Cluster seed / neighbour PE thresholds (ESA25 defaults 25/25).
    time_tolerance, pad_time : float
        Hit-time gate (hit dropped if time > pad_time + time_tolerance). Left
        wide open by default (real-data TS timing is not yet calibrated).
    calib_files : dict[int,str] | False | None
        None = vendored per-pad ESA25 calibration files; False = use scalar
        pedestal/gain; dict = explicit per-pad `channel gain pedestal` files.

    Returns
    -------
    (processors, cluster_collections)
        The ordered list of processors and the produced cluster collection names
        (one per pad, index-aligned to `pads`), ready to hand to
        TrigScintMeasurementProducer.input_collections.
    """
    from LDMX.TrigScint.trig_scint import (
        TrigScintClusterProducer,
        TrigScintRecHitProducer,
    )
    from LDMX.TrigScint.ts_daq_map import ts_calib_file_path

    def soi_for(pad):
        if sample_of_interest is None:
            return ESA25_SAMPLE_OF_INTEREST.get(pad, 8)
        if isinstance(sample_of_interest, dict):
            return sample_of_interest.get(pad, 8)
        return sample_of_interest

    def calib_for(pad):
        if calib_files is False:
            return None
        if calib_files is None:
            return ts_calib_file_path(pad)
        return calib_files.get(pad)

    processors = []
    cluster_collections = []
    for p in pads:
        rechit = TrigScintRecHitProducer(f"ts_rechits{p}")
        rechit.input_collection = f"decodedZCCMPad{p}"
        rechit.input_pass_name = input_pass
        rechit.output_collection = f"trigScintRecHitsPad{p}"
        rechit.pedestal = pedestal
        rechit.gain = gain
        rechit.mev_per_mip = mev_per_mip
        rechit.pe_per_mip = pe_per_mip
        rechit.sample_of_interest = soi_for(p)
        rechit.integration_window = integration_window
        calib = calib_for(p)
        if calib:
            rechit.use_calib_file = True
            rechit.calib_file = calib

        clusters = TrigScintClusterProducer(f"ts_clusters{p}")
        clusters.input_collection = rechit.output_collection
        clusters.output_collection = f"TrigScintClustersPad{p}"
        clusters.seed_threshold = seed_threshold
        clusters.clustering_threshold = clustering_threshold
        clusters.max_cluster_width = max_cluster_width
        clusters.time_tolerance = time_tolerance
        clusters.pad_time = pad_time

        processors += [rechit, clusters]
        cluster_collections.append(clusters.output_collection)

    return processors, cluster_collections
