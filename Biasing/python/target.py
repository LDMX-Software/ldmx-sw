"""Example configurations for producing biased interactions in the target.

    Example
    -------

        from LDMX.Biasing import target
"""

from LDMX.SimCore import generators
from LDMX.SimCore import simulator
from LDMX.SimCore import bias_operators
from LDMX.Biasing import filters
from LDMX.Biasing import util
from LDMX.Biasing import include as includeBiasing

def electro_nuclear( detector, generator ) :
    """Example configuration for producing electro-nuclear reactions in the target.

    In this particular example, 4 GeV electrons are fired upstream of the
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
    sim.setDetector( detector , True )

    # Set run parameters
    sim.runNumber = 0
    sim.description = "Target electron-nuclear, xsec bias 1e8"
    sim.beamSpotSmear = [20., 80., 0.] #mm

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.ElectroNuclear('target',1e8) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            filters.TargetENFilter(2500.),
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
    sim.setDetector( detector , True )

    # Set run parameters
    sim.runNumber = 0
    sim.description = "ECal photo-nuclear, xsec bias 450"
    sim.beamSpotSmear = [20., 80., 0.]

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('target',450.,2500.,only_children_of_primary=True) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(),
            filters.TargetPNFilter(),
            # Tag all photo-nuclear tracks to persist them to the event.
            util.TrackProcessFilter.photo_nuclear()
    ])

    return sim

def gamma_mumu( detector, generator ) :
    """Example configuration for biasing gamma to mu+ mu- conversions in the target.

    In this particular example, 4 GeV electrons are fired upstream of the
    tagger tracker.  The TargetBremFilter filters out all events that don't
    produced a brem in the target with an energy greater than 2.5 GeV. 

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
    sim.setDetector( detector , True )

    # Set run parameters
    sim.runNumber = 0
    sim.description = "gamma -> mu+ mu-, xsec bias 10e9"
    sim.beamSpotSmear = [20., 80., 0.]

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.GammaToMuPair('target', 1.E9, 2500.) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(),
    ])

    return sim

def dark_brem( ap_mass , lhe, detector ) :
    """Example configuration for producing dark brem interactions in the target.

    This configures the sim to fire a 4 GeV electron upstream of the
    tagger tracker.  The dark-photon production cross-section is biased up in
    the target.  Only events that result in a dark-photon being produced in the
    target are kept.

    Parameters
    ----------
    ap_mass : float
        The mass of the A' in MeV.
    lhe : str
        The path to the directory containing LHE files to use as events of the dark brem.
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
        target_ap_sim = target.dark_brem(1000., makePath.makeLHEPath(1000.), 'ldmx-det-v12')

    In general, the second argument should be the full path to the LHE event library.

        target_ap_sim = target.dark_brem(1000, 'path/to/lhe', 'ldmx-det-v12')
    """
    sim = simulator.simulator( "target_dark_brem_" + str(ap_mass) + "_MeV" )

    sim.description = "One e- fired far upstream with Dark Brem turned on and biased up in target"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    sim.beamSpotSmear = [ 20., 80., 0. ] #mm

    #Activiate dark bremming with a certain A' mass and LHE library
    from LDMX.SimCore import dark_brem
    db_model = dark_brem.VertexLibraryModel( lhe )
    db_model.threshold = 2. #GeV - minimum energy electron needs to have to dark brem
    db_model.epsilon   = 0.01 #decrease epsilon from one to help with Geant4 biasing calculations
    sim.dark_brem.activate( ap_mass , db_model )

    #Biasing dark brem up inside of the target
    #need to bias up high mass A' by more than 2 so that they can actually happen
    from math import log10
    mass_power = max(log10(sim.dark_brem.ap_mass),2.)
    sim.biasing_operators = [
            bias_operators.DarkBrem.target(sim.dark_brem.ap_mass**mass_power / db_model.epsilon**2)
            ]

    sim.actions.extend([
        #make sure electron reaches target with 3.5GeV
        filters.TaggerVetoFilter(3500.),
        #make sure dark brem occurs in the target where A' has at least 2GeV
        filters.TargetDarkBremFilter(2000.),
        #keep all prodcuts of dark brem(A' and recoil electron)
        util.TrackProcessFilter.dark_brem()
        ])

    return sim
