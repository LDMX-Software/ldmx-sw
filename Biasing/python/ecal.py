"""Example configurations for producing biased interactions in the ECal.

Example
-------

    from LDMX.Biasing import ecal
"""

from LDMX.Biasing import filters, util
from LDMX.SimCore import bias_operators, generators, simulator


def photo_nuclear(detector, generator):
    """Example configuration for producing photo-nuclear reactions in the ECal.

    In this particular example, 4 GeV electrons are fired upstream of the
    tagger tracker.  The TargetBremFilter filters out all events that don't
    produced a brem in the target with an energy greater than 2.5 GeV.  The
    brems are allowed to propagate to the ECal at which point they are
    checked by the EcalProcessFilter.  Only events that see the brem photon
    undergo a photo-nuclear reaction in the ECal are kept.

    Parameters
    ----------

    detector : str
        Path to the detector
    generator : PrimaryGenerator
        generator to use

    Returns
    -------
    Instance of the simulator configured for ECal photo-nuclear.

    Example
    -------

        ecal_pn_sim = ecal.photo_nuclear('ldmx-det-v12')

    """

    # Instantiate the simulator.
    sim = simulator.Simulator(instance_name="photo-nuclear")

    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.set_detector(detector, include_scoring_planes_minimal=True)

    # Set run parameters
    xsec_bias_threshold = 0.625 * generator.energy * 1000.0
    tagger_threshold = 0.95 * generator.energy * 1000.0
    recoil_max_p = 0.375 * generator.energy * 1000.0
    brem_min_e = 0.625 * generator.energy * 1000.0
    xsec_bias = 550.0 if generator.energy == 8.0 else 450.0

    sim.description = (
        "ECal photo-nuclear, xsec bias "
        + str(xsec_bias)
        + " xsec threshold "
        + str(xsec_bias_threshold)
        + " GeV"
    )

    sim.generators = [generator]

    # Enable and configure the biasing
    sim.biasing_operators = [
        bias_operators.PhotoNuclear(
            "ecal", xsec_bias, xsec_bias_threshold, only_children_of_primary=True
        )
    ]

    # Configure the sequence in which user actions should be called.
    sim.actions.extend(
        [
            filters.TaggerVetoFilter(threshold=tagger_threshold),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(
                recoil_max_p_threshold=recoil_max_p,
                brem_min_energy_threshold=brem_min_e,
            ),
            # Only consider events where a PN reaction happnes in the ECal
            filters.EcalProcessFilter(),
            # Tag all photo-nuclear tracks to persist them to the event.
            util.TrackProcessFilter.photo_nuclear(),
        ]
    )

    return sim


def nonfiducial_photo_nuclear(detector, generator):

    # Instantiate the simulator.
    sim = simulator.Simulator(instance_name="photo-nuclear")

    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.set_detector(detector, include_scoring_planes_minimal=True)

    # Set run parameters
    xsec_bias_threshold = 0.625 * generator.energy * 1000.0
    tagger_threshold = 0.95 * generator.energy * 1000.0
    recoil_max_p = 0.375 * generator.energy * 1000.0
    brem_min_e = 0.625 * generator.energy * 1000.0
    xsec_bias = 550.0 if generator.energy == 8.0 else 450.0

    sim.description = (
        "Non-fiducial ECal photo-nuclear, xsec bias "
        + str(xsec_bias)
        + " xsec threshold "
        + str(xsec_bias_threshold)
        + " GeV"
    )
    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [
        bias_operators.PhotoNuclear(
            "ecal", xsec_bias, xsec_bias_threshold, only_children_of_primary=True
        )
    ]

    # Configure the sequence in which user actions should be called.
    sim.actions.extend(
        [
            filters.TaggerVetoFilter(threshold=tagger_threshold),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(
                recoil_max_p_threshold=recoil_max_p,
                brem_min_energy_threshold=brem_min_e,
            ),
            # Only considers events that are Non-Fiducial (Doesn't enter an ECal volume)
            filters.NonFiducialFilter(recoil_max_p=recoil_max_p),
            # Only consider events where a PN reaction happens in the ECal
            filters.EcalProcessFilter(),
            # Tag all photo-nuclear tracks to persist them to the event.
            util.TrackProcessFilter.photo_nuclear(),
        ]
    )

    return sim


def gamma_mumu(detector, generator):
    """Example configuration for biasing gamma to mu+ mu- conversions in the ecal.

    In this particular example, 4 GeV elecrons are fired upstream of the
    tagger tracker. The TargetBremFilter filters out all events that don't
    produce a brem in the target with an energy greater than 2.5 GeV.

    Parameters
    ----------

    detector : str
        Path to the detector

    Returns
    -------
    Instance of the sim configured for target gamma to muon conversions.

    Example
    -------

        ecal_mumu_sim = ecal.gamma_mumu('ldmx-det-v12')

    """

    # Initiate the sim
    sim = simulator.Simulator(instance_name="ecal_gammamumu")

    # Set the path to the detector to use
    # Also tell the simulator to include scoring planes
    sim.set_detector(detector, include_scoring_planes_minimal=True)

    # Set run parameters
    xsec_bias_threshold = 0.625 * generator.energy * 1000.0
    tagger_threshold = 0.95 * generator.energy * 1000.0
    recoil_max_p = 0.375 * generator.energy * 1000.0
    brem_min_e = 0.625 * generator.energy * 1000.0
    xsec_bias = 100000.0 if generator.energy == 8.0 else 30000.0

    sim.description = (
        "gamma --> mu+ mu-, xsec bias "
        + str(xsec_bias)
        + " xsec threshold "
        + str(xsec_bias_threshold)
        + " GeV"
    )
    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [
        bias_operators.GammaToMuPair("ecal", xsec_bias, xsec_bias_threshold)
    ]

    # Configure the sequence in which user actions should be called
    sim.actions.extend(
        [
            filters.TaggerVetoFilter(threshold=tagger_threshold),
            filters.TargetBremFilter(
                recoil_max_p_threshold=recoil_max_p,
                brem_min_energy_threshold=brem_min_e,
            ),
            filters.EcalProcessFilter(process="GammaToMuPair"),
            util.TrackProcessFilter.gamma_mumu(),
        ]
    )

    return sim


def deep_photo_nuclear(
    detector,
    generator,
    bias_threshold,
    processes,
    ecal_min_z,
    require_photon_from_target=False,
):

    # Instantiate the simulator.
    sim = simulator.Simulator(instance_name="photo-nuclear")

    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.set_detector(detector, include_scoring_planes_minimal=True)

    # Set run parameters
    # Set run parameters
    xsec_bias_threshold = 0.625 * generator.energy * 1000.0
    xsec_bias = 550.0 if generator.energy == 8.0 else 450.0

    sim.description = (
        "deep ECal photo-nuclear, xsec bias "
        + str(xsec_bias)
        + " xsec threshold "
        + str(xsec_bias_threshold)
        + " GeV"
    )

    sim.generators = [generator]

    # Enable and configure the biasing
    sim.biasing_operators = [
        bias_operators.PhotoNuclear(
            "ecal",
            xsec_bias,
            xsec_bias_threshold,
            only_children_of_primary=True,
            down_bias_conv=False,
        )
    ]

    # Configure the sequence in which user actions should be called.
    sim.actions.extend(
        [
            # Make sure all particles above a given threshold are processed first
            util.PartialEnergySorter(0.8 * bias_threshold),
            # Make sure the primary electron with a given energy reaches the ECAL
            filters.PrimaryToEcalFilter(0.2 * generator.energy * 1000),
            # Only considers events when the interaction happend deep in the ECAL
            filters.DeepEcalProcessFilter(
                bias_threshold, processes, ecal_min_z, require_photon_from_target
            ),
            # Keep intersting particles in the output
            util.TrackProcessFilter.conversion(),
        ]
    )

    return sim
