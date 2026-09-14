"""Plotting of ECal-related validation plots"""

import logging

from ._differ import Differ
from ._plotter import plotter


log = logging.getLogger("ecal")


@plotter
def digi_verify(d: Differ, out_dir=None):
    """Plot ECal digi verify variables from the already created DQM histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    col = "EcalDigiVerify/num_sim_hits_per_cell"
    name = "Number of SimHits per ECal Cell (excluding empty rec cells)"
    log.info(f"plotting {col}")
    d.plot1d(col, name, out_dir=out_dir, legend_kw={"loc": "upper left"})

    features = [
        ("EcalDigiVerify/num_rec_hits", "Number of RecHits"),
        ("EcalDigiVerify/num_noise_hits", "Number of noisy RecHits"),
        ("EcalDigiVerify/total_rec_energy", "Total Reconstructed Energy in ECal [MeV]"),
    ]
    for col, name in features:
        log.info(f"plotting {col}")
        rebin = 10 if "total_rec_energy" in col else 1
        d.plot1d(col, name, out_dir=out_dir, rebin=rebin)


@plotter
def shower_feats(d: Differ, out_dir=None):
    """Plot ECal shower features from the already created DQM histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    col, name = "EcalShowerFeatures/deepest_layer_hit", "Deepest Layer Hit"
    log.info(f"plotting {col}")
    d.plot1d(col, name, out_dir=out_dir, legend_kw={"loc": "upper left"})

    features = [
        ("EcalShowerFeatures/num_readout_hits", "N Readout Hits"),
        ("EcalShowerFeatures/summed_det", "Total Rec Energy [MeV]"),
        ("EcalShowerFeatures/summed_iso", "Total Isolated Energy [MeV]"),
        ("EcalShowerFeatures/summed_back", "Total Back Energy [MeV]"),
        ("EcalShowerFeatures/max_cell_dep", "Max Cell Dep [MeV]"),
        ("EcalShowerFeatures/shower_rms", "Shower RMS [mm]"),
        ("EcalShowerFeatures/x_std", "X Standard Deviation [mm]"),
        ("EcalShowerFeatures/y_std", "Y Standard Deviation [mm]"),
        ("EcalShowerFeatures/avg_layer_hit", "Avg Layer Hit"),
        ("EcalShowerFeatures/std_layer_hit", "Std Dev Layer Hit"),
    ]
    for col, name in features:
        log.info(f"plotting {col}")
        d.plot1d(col, name, out_dir=out_dir)


@plotter
def mip_tracking(d: Differ, out_dir=None):
    """Plot ECal MIP tracking features from the already created DQM histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    col, name = "EcalMipTrackingFeatures/n_straight_tracks", "Number of Straight Tracks"
    log.info(f"plotting {col}")
    d.plot1d(col, name, out_dir=out_dir, legend_kw={"loc": "upper left"})

    features = [
        (
            "EcalMipTrackingFeatures/n_linreg_segments",
            "Number of Linear Regression Segments",
        ),
        ("EcalMipTrackingFeatures/first_near_photon_layer", "First Near Photon Layer"),
        ("EcalMipTrackingFeatures/ep_ang", "Electron Photon Angle [degree]"),
        ("EcalMipTrackingFeatures/ep_sep", "Electron Photon Separation"),
        ("EcalMipTrackingFeatures/recoil_pz", "Recoil electron pz [MeV]"),
        ("EcalMipTrackingFeatures/recoil_pt", "Recoil electron pT [MeV]"),
        ("EcalMipTrackingFeatures/recoil_x", "Recoil electron x[mm]"),
        ("EcalMipTrackingFeatures/recoil_y", "Recoil electron y [mm]"),
    ]
    for col, name in features:
        log.info(f"plotting {col}")
        d.plot1d(col, name, out_dir=out_dir)


@plotter
def veto_results(d: Differ, out_dir=None):
    """Plot ECAL veto results from the already created DQM histograms

    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    col, name = "EcalVetoResults/bdt_disc", "BDT discriminating score"
    log.info(f"plotting {col}")
    d.plot1d(col, name, out_dir=out_dir, legend_kw={"loc": "upper left"})

    features = [
        ("EcalVetoResults/bdt_disc_log", "-log(1-BDT discriminating score)"),
        ("EcalVetoResults/fiducial", "Recoil eletron fiducial"),
    ]
    for col, name in features:
        log.info(f"plotting {col}")
        d.plot1d(col, name, out_dir=out_dir)


@plotter
def clue_cluster(d: Differ, out_dir=None):
    """
    Plot ECal CLUE cluster features from the already created DQM histograms
    Parameters
    ----------
    d : Differ
        Differ containing files that are not event files (presumably histogram files)
    """

    features = [
        (
            "EcalClusterAnalyzer/number_of_clusters_first_layer",
            "Number of CLUE clusters on the first layer",
        ),
        (
            "EcalClusterAnalyzer/number_of_clusters_per_layer",
            "Number of CLUE clusters per layer",
        ),
        ("EcalClusterAnalyzer/number_of_clusters", "Total number of CLUE clusters"),
        ("EcalClusterAnalyzer/energy_percentage", "Percentage of energy in cluster"),
        (
            "EcalClusterAnalyzer/clusterless_hits_percentage",
            "Percentage of hits not in a cluster",
        ),
        (
            "EcalClusterAnalyzer/sp_clue_distance",
            "CLUE centroid to SP ele distance in xy-plane [mm]",
        ),
        ("EcalClusterAnalyzer/sp_clue_x_residual", "CLUE centroid X - SP ele X [mm]"),
        ("EcalClusterAnalyzer/sp_clue_y_residual", "CLUE centroid Y - SP ele Y [mm]"),
        ("EcalClusterAnalyzer/correctly_predicted_events", ""),
    ]
    for col, name in features:
        log.info(f"plotting {col}")
        d.plot1d(col, name, out_dir=out_dir)
