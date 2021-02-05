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

def midshower_nuclear( detector , bias_factor , bias_threshold , min_nuclear_energy ) :
    """Example configuration for producing mid-shower nuclear interactions in
    the ECal that fake a missing energy (ME) signal.
       
    In this particular example, 4 GeV electrons are fired upstream of the 
    tagger tracker. Then simulation events are then put through a series of
    filters.

    Parameters
    ----------

    detector : str
        Name of the detector
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
        eat_pn_sim = eat.midshower_nuclear('ldmx-det-v12',1000.,1700.)

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
    
    from LDMX.SimCore import generators
    sim.generators = [ generators.single_4gev_e_upstream_tagger() ]

    #Enable and configure the biasing
    from LDMX.SimCore import bias_operators
    sim.biasing_operators = [ 
            bias_operators.PhotoNuclear('ecal',bias_factor,bias_threshold),
            bias_operators.ElectroNuclear('ecal',bias_factor,bias_threshold)
            ]

    #Configure the sequence in which user actions should be called.
    sim.actions = [
            #get electron to ECal with 3500MeV
            filters.PrimaryToEcalFilter(3500.),
            #Make sure all particles above 1500MeV are processed first
            util.PartialEnergySorter(bias_threshold),
            #Make sure a total of 1700MeV energy went PN in ECal
            filters.MidShowerNuclearBkgdFilter(min_nuclear_energy),
    ]

    return sim

def dark_brem( ap_mass , lhe, detector ) :
    """Example configuration for producing dark brem interactions in the ECal. 

    This configures the simulator to fire a 4 GeV electron upstream of the 
    tagger tracker.  The electron is allowed to propagate into the ECal where 
    the dark-photon production cross-section is biased up.  Only events that 
    result in a dark-photon being produced in the ECal are kept. 

    Parameters
    ----------
    ap_mass : float
        The mass of the A' in MeV.
    lhe : str
        The path to the LHE file to use as vertices of the dark brem. 
    detector : str
        Path to the detector.

    Return
    ------
    Instance of the simulator configured for dark-brem production in the ECal.

    Example
    -------
    Here we use the example vertex library. This should not be used
    for large (>50k) event samples.

        from LDMX.SimCore import makePath
        eat_ap_sim = eat.dark_brem(1000., makePath.makeLHEPath(1000.), 'ldmx-det-v12')

    """
    
    sim = simulator.simulator( "ecal_dark_brem_%sMeV" % str(ap_mass) )
    from LDMX.Ecal import EcalGeometry
    
    sim.description = "One e- fired far upstream with Dark Brem turned on and biased up in ECal"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    sim.beamSpotSmear = [ 20., 80., 0. ] #mm

    #Activiate dark bremming with a certain A' mass and LHE library
    from LDMX.SimCore import dark_brem
    db_model = dark_brem.VertexLibraryModel( lhe )
    db_model.threshold = 2. #GeV - minimum energy electron needs to have to dark brem
    db_model.epsilon   = 0.01 #decrease epsilon from one to help with Geant4 biasing calculations
    sim.dark_brem.activate( ap_mass , db_model )

    #Biasing dark brem up inside of the ecal volumes
    from math import log10
    #need a higher power for the higher mass A'
    mass_power = max(log10(sim.dark_brem.ap_mass),2.)

    from LDMX.SimCore import bias_operators
    sim.biasing_operators = [ 
            bias_operators.DarkBrem.ecal(sim.dark_brem.ap_mass**mass_power / db_model.epsilon**2)
            ]
    
    sim.actions = [
            #Make sure all particles above 2GeV are processed first
            util.PartialEnergySorter(2000.),
            #Abort events if the electron doesn't get to the ECal with 3.5GeV
            filters.PrimaryToEcalFilter(3500.),
            #Only keep events when a dark brem happens in the Ecal
            filters.EcalDarkBremFilter(2000.)
    ]
    
    return sim
