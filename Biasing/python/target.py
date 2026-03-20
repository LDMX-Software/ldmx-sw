"""Example configurations for producing biased interactions in the target.

    Example
    -------

        from LDMX.Biasing import target
"""

from LDMX.Biasing import filters, util
from LDMX.Biasing import include as includeBiasing
from LDMX.SimCore import bias_operators, generators, simulator


def electro_nuclear( detector, generator ) :
    """Example configuration for producing electro-nuclear reactions in the target.

    In this particular example, 8 GeV electrons are fired upstream of the
    tagger tracker. TargetENFilter filters out events that don't see an
    electro-nuclear reaction take places in the target.

    Parameters
    ----------

    detector : str
        Path to the detector

    Returns
    -------
    Instance of the sim configured for target electro-nuclear.

    Example
    -------

        target_en_sim = target.electro_nuclear('ldmx-det-v12')

    """

    # Instantiate the sim.
    sim = simulator.simulator("target_electronNuclear")

    # Set the path to the detector to use.
    #   Also tell the simulator to include scoring planes
    sim.setDetector( detector , include_scoring_planes_minimal = True )

    # Set run parameters
    sim.description = "Target electron-nuclear, xsec bias 1e5"

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.ElectroNuclear('target',1e5) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    recoil_thresh = 0.975 * generator.energy * 1000.
    tagger_threshold = 0.95 * generator.energy * 1000.
    sim.actions.extend([
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            filters.TargetENFilter(recoil_thresh),
            util.TrackProcessFilter.electro_nuclear()
    ])

    return sim

def photo_nuclear( detector, generator ) :
    """Example configuration for producing photo-nuclear reactions in the ECal.

    In this particular example, 4 GeV electrons are fired upstream of the
    tagger tracker.  The TargetBremFilter filters out all events that don't
    produced a brem in the target with an energy greater than 2.5 GeV.
    TargetBremFilter filters out events that don't see the brem photon undergo
    a photo-nuclear reaction in the target.

    Parameters
    ----------

    detector : str
        Path to the detector

    Returns
    -------
    Instance of the sim configured for target photo-nuclear.

    Example
    -------

        target_pn_sim = target.photo_nuclear('ldmx-det-v12')

    """


    # Instantiate the sim.
    sim = simulator.simulator("target_photonNuclear")

    # Set the path to the detector to use.
    #   Also tell the simulator to include scoring planes
    sim.setDetector( detector , include_scoring_planes_minimal = True )

    # Set run parameters
    xsec_bias_threshold = 0.625 * generator.energy * 1000.
    tagger_threshold = 0.95 * generator.energy * 1000.
    recoil_max_p = 0.375 * generator.energy * 1000.
    brem_min_e = 0.625 * generator.energy * 1000.
    xsec_bias = 550. if generator.energy == 8.0 else 450.

    sim.description = (
        f"Target photo-nuclear, xsec bias {xsec_bias}"
        f" xsec threshold {xsec_bias_threshold} GeV"
    )

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('target',
    xsec_bias,xsec_bias_threshold,only_children_of_primary=True) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(recoil_max_p = recoil_max_p,
            brem_min_e = brem_min_e),
            filters.TargetPNFilter(),
            # Tag all photo-nuclear tracks to persist them to the event.
            util.TrackProcessFilter.photo_nuclear()
    ])

    return sim

def gamma_mumu( detector, generator ) :
    """Example configuration for biasing gamma to mu+ mu- target conversions

    In this particular example, 8 GeV electrons are fired upstream of the
    tagger tracker.  The TargetBremFilter filters out all events that don't
    produced a brem in the target with an energy greater than 5 GeV.

    Parameters
    ----------

    detector : str
        Path to the detector

    Returns
    -------
    Instance of the sim configured for target gamma to muon conversions.

    Example
    -------

        target_mumu_sim = target.gamma_mumu('ldmx-det-v12')

    """

    # Instantiate the sim.
    sim = simulator.simulator("target_gammamumu")

    # Set the path to the detector to use.
    #   Also tell the simulator to include scoring planes
    sim.setDetector( detector , include_scoring_planes_minimal = True )

    # Set run parameters
    xsec_bias_threshold = 0.625 * generator.energy * 1000.
    tagger_threshold = 0.95 * generator.energy * 1000.
    recoil_max_p = 0.375 * generator.energy * 1000.
    brem_min_e = 0.625 * generator.energy * 1000.
    xsec_bias = 1.E5 if generator.energy == 8.0 else 3.E4

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.description = (
        f"gamma --> mu+ mu-, xsec bias {xsec_bias}"
        f" xsec threshold {xsec_bias_threshold} GeV"
    )
    sim.biasing_operators = [
        bias_operators.GammaToMuPair(
            'target', xsec_bias, xsec_bias_threshold
        )
    ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            # Only consider events where a hard brem occurs
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            filters.TargetBremFilter(
                recoil_max_p=recoil_max_p,
                brem_min_e=brem_min_e,
            ),
            filters.TargetGammaMuMuFilter(),
            util.TrackProcessFilter.gamma_mumu()
    ])

    return sim

def dark_brem( ap_mass , lhe, detector, generator,
             scale_aprime = False, decay_mode = 'no_decay',
             ap_tau = -1.0, dist_decay_min = 0.0,
             dist_decay_max = 1.0) :
    """Example configuration for producing dark brem interactions in the target.

    This configures the sim to fire a 8 GeV electron upstream of the
    tagger tracker.  The dark-photon production cross-section is biased up in
    the target.  Only events that result in a dark-photon being produced in the
    target are kept.

    Parameters
    ----------
    ap_mass : float
        The mass of the A' in MeV.
    lhe : str
        The path to the directory containing LHE files to use as events
        of the dark brem.
    detector : str
        Name of detector to simulate in

    Return
    ------
    Instance of the sim configured for dark-brem production in the target.

    Example
    -------
    Here we use the example vertex library. This should not be used
    for large (>50k) event samples.

        from LDMX.SimCore import makePath
        target_ap_sim = target.dark_brem(1000., makePath.makeLHEPath(1000.),
        'ldmx-det-v12')

    In general, the second argument should be the full path to the LHE event library.

        target_ap_sim = target.dark_brem(1000, 'path/to/lhe', 'ldmx-det-v12')
    """
    sim = simulator.simulator( "target_dark_brem_" + str(ap_mass) + "_MeV" )

    sim.description = (
        f"One e- fired far upstream with Dark Brem turned on and biased up in target"
        f" A' mass {ap_mass} MeV"
    )
    sim.setDetector( detector , include_scoring_planes_minimal = True )
    sim.generators.append( generator )

    #Activiate dark bremming with a certain A' mass and LHE library
    from LDMX.SimCore import dark_brem
    db_model = dark_brem.G4DarkBreMModel(lhe)
    # GeV - minimum energy electron needs to have to dark brem
    db_model.threshold = 4.
    # decrease epsilon from one to help with Geant4 biasing calculations
    db_model.epsilon   = 0.01
    db_model.scale_aprime = scale_aprime
    db_model.decay_mode = decay_mode
    db_model.ap_tau = ap_tau
    db_model.dist_decay_min = dist_decay_min
    db_model.dist_decay_max = dist_decay_max
    sim.dark_brem.activate( ap_mass , db_model )

    import math
    mass_power = max(math.log10(sim.dark_brem.ap_mass), 2.)

    #Biasing dark brem up inside of the target
    sim.biasing_operators = [
            bias_operators.DarkBrem.target(
                sim.dark_brem.ap_mass**mass_power / db_model.epsilon**2)
            ]

    sim.actions.extend([
        #make sure electron reaches target with 7 GeV
        filters.TaggerVetoFilter(7000.),
        #make sure dark brem occurs in the target where A' has at least 4GeV
        filters.TargetDarkBremFilter(4000.),
        #keep all prodcuts of dark brem(A' and recoil electron)
        util.TrackProcessFilter.dark_brem()
        ])

    return sim


def aprime_to_fcp( ap_mass, fcp_mass, lhe, detector, generator,
                   fcp_charge = 0.1,
                   fcp_xsec_factor = 1e10,
                   scale_aprime = False, decay_mode = 'no_decay',
                   ap_tau = -1.0, dist_decay_min = 0.0,
                   dist_decay_max = 1.0) :
    """Example configuration for producing A' -> fcp+ fcp- in the target.

    This configures the sim to fire an 8 GeV electron upstream of the
    tagger tracker. The dark-photon production cross-section is biased up in
    the target. The A' then converts to a pair of fractionally charged
    particles (fcp+ fcp-) via the APrimeToFCPPair process.

    Parameters
    ----------
    ap_mass : float
        The mass of the A' in MeV (should be small / near zero).
    fcp_mass : float
        The mass of the fcp in MeV.
    lhe : str
        The path to the directory containing LHE files to use as events
        of the dark brem.
    detector : str
        Name of detector to simulate in.
    generator : PrimaryGenerator
        Generator to use for the primary particles.
    fcp_charge : float, optional
        Electric charge of the fcp in units of e (default: 0.1).
    fcp_xsec_factor : float, optional
        Cross section biasing factor for A' -> fcp conversion.
        The physical cross section scales as q^4, so for q=0.1e
        a factor ~1e8 is needed for conversions (default: 1e8).
    scale_aprime : bool, optional
        Should we scale the A' (default: False).
    decay_mode : str, optional
        Decay mode for the A' (default: 'no_decay').
    ap_tau : float, optional
        Proper lifetime of the A' (default: -1.0).
    dist_decay_min : float, optional
        Minimum decay distance (default: 0.0).
    dist_decay_max : float, optional
        Maximum decay distance (default: 1.0).

    Return
    ------
    Instance of the sim configured for A' -> fcp+ fcp- production in the target.

    Example
    -------

        from LDMX.SimCore import makePath
        target_fcp_sim = target.aprime_to_fcp(
            0.01, 100., makePath.makeLHEPath(0.01), 'ldmx-det-v15-8gev',
            generators.single_8gev_e_upstream_tagger())
    """
    sim = simulator.simulator( "target_aprime_fcp_" + str(fcp_mass) + "_MeV" )

    sim.description = (
        f"One e- fired far upstream with Dark Brem turned on and biased up in target,"
        f" A' mass {ap_mass} MeV, fcp mass {fcp_mass} MeV, fcp charge {fcp_charge}e"
    )
    sim.setDetector( detector , include_scoring_planes_minimal = True )
    sim.generators.append( generator )

    # Activate dark bremming with a certain A' mass and LHE library
    from LDMX.SimCore import dark_brem
    db_model = dark_brem.G4DarkBreMModel(lhe)
    db_model.threshold = 4.  # GeV
    db_model.epsilon   = 0.01
    db_model.scale_aprime = scale_aprime
    db_model.decay_mode = decay_mode
    db_model.ap_tau = ap_tau
    db_model.dist_decay_min = dist_decay_min
    db_model.dist_decay_max = dist_decay_max
    sim.dark_brem.activate( ap_mass , db_model )

    # Activate A' -> fcp+ fcp- conversion with cross section biasing
    sim.dark_brem.activate_fcp( fcp_mass , fcp_charge, fcp_xsec_factor )

    import math
    mass_power = max(math.log10(sim.dark_brem.ap_mass), 2.)

    # Biasing dark brem up inside of the target
    sim.biasing_operators = [
            bias_operators.DarkBrem.target(
                sim.dark_brem.ap_mass**mass_power / db_model.epsilon**2)
            ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    sim.actions.extend([
        # make sure electron reaches target with 7 GeV
        filters.TaggerVetoFilter(7000.),
        # make sure dark brem occurs in the target where A' has at least 4GeV
        filters.TargetDarkBremFilter(4000.),
        # keep all products of dark brem and fcp conversion
        util.TrackProcessFilter.dark_brem(),
        util.TrackProcessFilter.aprime_to_fcp()
        ])

    return sim

def gamma_to_fcp( detector, generator, fcp_mass, fcp_charge = 0.1 ) :
    """Example configuration for biasing gamma to fcp+ fcp- target conversions

    In this particular example, electrons are fired upstream of the
    tagger tracker. The TargetBremFilter filters out all events that don't
    produce a brem in the target with sufficient energy. The gamma -> fcp
    conversion process is then biased up.

    This is the SM-photon analogue of gamma_mumu(), producing fractionally
    charged particles instead of muons.

    Parameters
    ----------
    detector : str
        Path to the detector
    generator : PrimaryGenerator
        Generator to use for the primary particles.
    fcp_mass : float
        The mass of the fcp in MeV.
    fcp_charge : float, optional
        Electric charge of the fcp in units of e (default: 0.1).

    Returns
    -------
    Instance of the sim configured for target gamma to fcp conversions.

    Example
    -------

        target_fcp_sim = target.gamma_to_fcp('ldmx-det-v15-8gev',
            generators.single_8gev_e_upstream_tagger(), 100.0)

    """

    # Instantiate the sim.
    sim = simulator.simulator("target_gamma_fcp")

    # Set the path to the detector to use.
    #   Also tell the simulator to include scoring planes
    sim.setDetector( detector , include_scoring_planes_minimal = True )

    # Set run parameters
    xsec_bias_threshold = min(0.625 * generator.energy * 1000., 2. * fcp_mass)
    tagger_threshold = 0.95 * generator.energy * 1000.
    
    brem_min_e = min(0.625 * generator.energy * 1000., 2. * fcp_mass)
    recoil_max_p = generator.energy * 1000. - brem_min_e
    xsec_bias = 1.E10 if generator.energy == 8.0 else 3.E9

    sim.generators.append(generator)

    # Enable gamma -> fcp conversion via the independent FCPPhysics constructor
    sim.fcp_physics.activate( fcp_mass, fcp_charge )

    # Enable and configure the biasing
    sim.description = (
        f"gamma --> fcp+ fcp-, xsec bias {xsec_bias}"
        f" xsec threshold {xsec_bias_threshold} MeV"
        f" fcp mass {fcp_mass} MeV, fcp charge {fcp_charge}e"
    )
    sim.biasing_operators = [
        bias_operators.GammaToFCPPair(
            'target', xsec_bias, xsec_bias_threshold
        )
    ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            # Only consider events where a hard brem occurs
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            filters.TargetBremFilter(
                recoil_max_p=recoil_max_p,
                brem_min_e=brem_min_e,
            ),
            filters.TargetGammaFCPFilter(),
            util.TrackProcessFilter.gamma_to_fcp()
    ])

    return sim
