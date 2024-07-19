"""Example configurations for producing biased interactions in the ECal. 

This module is built for the ECal as Target (EaT) analysis channel where
we are studying the events that have a primary electron reaching the ECal
at nearly full energy.

Example
-------
    
    from LDMX.Biasing import eat
"""

from LDMX.SimCore import simulator
from LDMX.SimCore import generators
from LDMX.Biasing import filters
from LDMX.Biasing import util

def midshower_nuclear( detector , generator, bias_factor , bias_threshold , min_nuclear_energy ) :
    """Example configuration for producing mid-shower nuclear interactions in
    the ECal that fake a missing energy (ME) signal.
       
    In this particular example, 4 GeV electrons are fired upstream of the 
    tagger tracker. Then simulation events are then put through a series of
    filters.

    Parameters
    ----------

    detector : str
        Name of the detector
    generator : simcfg.PrimaryGenerator
        Beam generator for this simulation which should be a ParticleGun
        so we can configure the PrimaryToEcalFilter to select events where
        beam electrons retain 87.5% of their energy.
    bias_factor : float
        Factor to multiply EN/PN xsecs by
    bias_threshold : float
        Minium energy [MeV] to bias a particle
    min_nuclear_energy : float
        Minium total energy [MeV] that went nuclear to keep event

    Returns
    -------
    simulator :
        configured for mid-shower nuclear interactions and far-upstream generator

    Example
    -------

        from LDMX.Biasing import eat
        from LDMX.SimCore import generators
        eat_dimuon = eat.midshower_nuclear(
            'ldmx-det-v14',
            generators.single_e_4gev_upstream_tagger(),
            200,
            1500.,
            2500.
        )

    """

    #Instantiate the simulator. 
    sim = simulator.simulator("eat-midshower-nuclear")
    from LDMX.Ecal import EcalGeometry

    #Set the path to the detector to use.
    #the second parameter says we want to include scoring planes
    sim.setDetector( detector , True )

    #Set run parameters
    sim.description = "Biased Mid-Shower Nuclear Interactions ME Background"
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators = [ generator ]

    #Enable and configure the biasing
    from LDMX.SimCore import bias_operators
    sim.biasing_operators = [ 
            bias_operators.PhotoNuclear('ecal',bias_factor,bias_threshold),
            bias_operators.ElectroNuclear('ecal',bias_factor,bias_threshold)
            ]

    #Configure the sequence in which user actions should be called.
    sim.actions = [
            filters.PrimaryToEcalFilter(0.875*generator.energy*1000),
            #Make sure all particles above 1500MeV are processed first
            util.PartialEnergySorter(bias_threshold),
            #Make sure a total of 1700MeV energy went PN in ECal
            filters.MidShowerNuclearBkgdFilter(min_nuclear_energy),
    ]

    return sim


def midshower_dimuon( detector , generator, bias_factor , bias_threshold , min_dimuon_energy ) :
    """Example configuration for producing mid-shower dimuon interactions in
    the ECal that fake a missing energy (ME) signal.
       
    In this particular example, 4 GeV electrons are fired upstream of the 
    tagger tracker. Then simulation events are then put through a series of
    filters.

    Parameters
    ----------

    detector : str
        Name of the detector
    generator : simcfg.PrimaryGenerator
        Beam generator for this simulation which should be a ParticleGun
        so we can configure the PrimaryToEcalFilter to select events where
        beam electrons retain 87.5% of their energy.
    bias_factor : float
        Factor to multiply GammaToMuPair xsec by within the ECal
    bias_threshold : float
        Minium energy [MeV] to bias a photon
    min_dimuon_energy : float
        Minium total energy [MeV] that went to muons to keep event

    Returns
    -------
    simulator :
        configured for mid-shower dimuon interactions and far-upstream generator

    Example
    -------

        from LDMX.Biasing import eat
        from LDMX.SimCore import generators
        eat_dimuon = eat.midshower_dimuon(
            'ldmx-det-v14',
            generators.single_e_4gev_upstream_tagger(),
            1e4,
            500.,
            1000.
        )

    """

    #Instantiate the simulator. 
    sim = simulator.simulator("eat-midshower-dimuon")
    from LDMX.Ecal import EcalGeometry

    #Set the path to the detector to use.
    #the second parameter says we want to include scoring planes
    sim.setDetector( detector , True )

    #Set run parameters
    sim.description = "Biased Mid-Shower DiMuon Interactions ME Background"
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    from LDMX.SimCore import generators
    sim.generators = [ generator ]

    #Enable and configure the biasing
    from LDMX.SimCore import bias_operators
    sim.biasing_operators = [ bias_operators.GammaToMuPair('ecal', bias_factor, bias_threshold) ]

    #Configure the sequence in which user actions should be called.
    sim.actions = [
            filters.PrimaryToEcalFilter(0.875*generator.energy*1000),
            util.PartialEnergySorter(bias_threshold),
            filters.MidShowerDiMuonBkgdFilter(min_dimuon_energy),
    ]

    return sim


def dark_brem(ap_mass, db_event_lib, detector, generator, 
              scale_APrime = False, decay_mode = 'no_decay',
              ap_tau = -1.0, dist_decay_min = 0.0,
              dist_decay_max = 1.0) :
    """Example configuration for producing dark brem interactions in the ECal. 

    This configures the simulator to fire a 4 GeV electron upstream of the 
    tagger tracker.  The electron is allowed to propagate into the ECal where 
    the dark-photon production cross-section is biased up.  Only events that 
    result in a dark-photon being produced in the ECal are kept. 

    Parameters
    ----------
    ap_mass : float
        The mass of the A' in MeV.
    db_event_lib : str
        The path to the reference library to use as vertices of the dark brem. 
    detector : str
        Path to the detector.
    generator : simcfg.PrimaryGenerator
        Beam generator for this simulation which should be a ParticleGun
        so we can configure the PrimaryToEcalFilter to select events where
        beam electrons retain 87.5% of their energy.
    scale_APrime : bool
        Whether to scale the A' momentum along with the recoil electron.
    decay_mode : str
        The A' decay mode. Either no_decay, flat_decay, or geant_decay
    ap_tau : float
        The A' decay lifetime in seconds
    dist_decay_min : float
        The minimum lab-frame distance at which to decay the A'
    dist_decay_max : float
        The maximum lab-frame distance at which to decay the A'

    Return
    ------
    Instance of the simulator configured for dark-brem production in the ECal.

    Example
    -------

        from LDMX.SimCore import generators
        eat_ap_sim = eat.dark_brem(
            1000.,
            'path/to/1GeV_mA_library',
            'ldmx-det-v14',
            generators.single_e_4gev_upstream_tagger()
        )

    """
    
    sim = simulator.simulator( "ecal_dark_brem_%sMeV" % str(ap_mass) )
    from LDMX.Ecal import EcalGeometry
    
    sim.description = "One e- fired far upstream with Dark Brem turned on and biased up in ECal"
    sim.setDetector( detector , True )
    sim.generators = [ generator ]
    sim.beamSpotSmear = [ 20., 80., 0. ] #mm

    #Activiate dark bremming with a certain A' mass and LHE library
    from LDMX.SimCore import dark_brem
    db_model = dark_brem.G4DarkBreMModel( db_event_lib )
    db_model.threshold = 0.5*generator.energy #GeV - minimum energy electron needs to have to dark brem
    db_model.epsilon   = 0.01 #decrease epsilon from one to help with Geant4 biasing calculations
    db_model.scale_APrime = scale_APrime
    db_model.decay_mode = decay_mode
    db_model.ap_tau = ap_tau
    db_model.dist_decay_min = dist_decay_min
    db_model.dist_decay_max = dist_decay_max
    sim.dark_brem.activate( ap_mass , db_model )

    #Biasing dark brem up inside of the ecal volumes
    from LDMX.SimCore import bias_operators
    from math import log10
    sim.biasing_operators = [ 
            bias_operators.DarkBrem.ecal(
                sim.dark_brem.ap_mass**max(2, log10(sim.dark_brem.ap_mass)) / db_model.epsilon**2
              )
            ]
    
    beam_energy = generator.energy*1000
    sim.actions = [
            util.DecayChildrenKeeper([622]), # keep children of A' decay
            util.PartialEnergySorter(0.5*beam_energy),
            filters.PrimaryToEcalFilter(0.875*beam_energy),
            filters.EcalDarkBremFilter(0.5*beam_energy)
    ]
    
    return sim
