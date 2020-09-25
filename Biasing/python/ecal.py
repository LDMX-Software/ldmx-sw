"""Example configurations for producing biased interactions in the ECal. 

    Example
    -------
        
        from LDMX.Biasing import ecal
"""

from LDMX.SimCore import simulator
from LDMX.SimCore import generators
from LDMX.Biasing import filters
from LDMX.Biasing import include as includeBiasing

def photo_nuclear( detector, generator ) :
    """Example configuration for producing photo-nuclear reactions in the ECal.  
       
    In this particular example, 4 GeV electrons are fired upstream of the 
    tagger tracker.  The TargetBremFilter filters out all events that don't 
    produced a brem in the target with an energy greater than 2.5 GeV.  The
    brems are allowed to propagate to the ECal at which point they are 
    checked by the EcalProcessFilter.  Only events that see the brem photon
    undergo a photo-nucler reaction in the ECal are kept. 

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
    sim = simulator.simulator("photo-nuclear")
    
    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.setDetector( detector , True )
    
    # Set run parameters
    sim.runNumber = 0
    sim.description = "ECal photo-nuclear, xsec bias 450"
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasingOn()
    sim.biasingConfigure( 'photonNuclear' , 'ecal' , 2500. , 450. )

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(),
            # Only consider events where a PN reaction happnes in the ECal
            filters.EcalProcessFilter(),     
            # Tag all photo-nuclear tracks to persist them to the event.
            filters.TrackProcessFilter.photo_nuclear()
    ])

    return sim

def electro_nuclear(detector) :
    """Example configuration for producing EN interactions in the ECal

    The primary electron is allowed to propagate into the ECal where 
    the EN interaction is biased up.  Only events that 
    result in a EN products totaling more than 2GeV in energy are kept. 

    Parameters
    ----------
    detector : str
        Name of detector to use

    Returns
    -------
    Instance of the simulator configured for ECal electro-nuclear.

    Example
    -------

        ecal_en_sim = ecal.electro_nuclear('ldmx-det-v12')

    """
    
    sim = simulator.simulator( "ecal_electron_nuclear" )
    
    sim.description = "One e- fired far upstream with electron nuclear interactions biased up in ECal"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    sim.beamSpotSmear = [ 20., 80., 0. ] #mm
    
    # Biasing dark brem up inside of the ecal volumes
    sim.biasingOn()
    sim.biasingConfigure( 'electronNuclear' , 'ecal' , 2000. , 4.5e4 , 
                allPtl = True, incidentOnly = False)
    
    # the following filters are in a library that needs to be included
    from LDMX.Biasing import include
    include.library()

    sim.actions.extend([ 
            # Abort events if the electron doesn't get to teh ECal with 3.5GeV
            filters.PrimaryToEcalFilter( 3500. ),
            # Abort events if primary electron doesn't undergo EN interactions totaling 2000MeV
            filters.EcalENFilter(2000.),
            # Keep all of the EN secondaries
            filters.TrackProcessFilter.electro_nuclear()
    ])
    
    return sim

def brem_pn(detector) :
    """Example configuration for producing a hard brem and a PN interaction in the ECal

    The primary electron is allowed to propagate into the ECal where 
    the PN interaction is biased up.  Only events that result in
    a hard brem (E_gamma > 2GeV) going PN are kept.

    Parameters
    ----------
    detector : str
        Name of detector to use

    Returns
    -------
    Instance of the simulator configured for ECal brem and photo-nuclear.

    Example
    -------

        ecal_brem_pn_sim = ecal.brem_pn('ldmx-det-v12')

    """
    
    sim = simulator.simulator( "ecal_brem_then_pn" )
    
    sim.description = "One e- fired far upstream with photon-nuclear interactions biased up in ECal"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    sim.beamSpotSmear = [ 20., 80., 0. ] #mm
    
    # Biasing dark brem up inside of the ecal volumes
    sim.biasingOn()
    sim.biasingConfigure( 'photonNuclear' , 'ecal' , 2000. , 4.5e2 , 
                allPtl = True, incidentOnly = False)
    
    # the following filters are in a library that needs to be included
    from LDMX.Biasing import include
    include.library()

    sim.actions.extend([ 
            # Abort events if the electron doesn't get to teh ECal with 3.5GeV
            filters.PrimaryToEcalFilter(3500.),
            # Abort events if primary electron doesn't hard brem (E_gamma > 2000MeV)
            filters.EcalBremFilter(2000.),
            # Make sure brem photon undergoes PN
            filters.EcalProcessFilter(),
            # Keep all of the PN secondaries
            filters.TrackProcessFilter.photo_nuclear()
    ])
    
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
        ecal_ap_sim = ecal.dark_brem(1000., makePath.makeLHEPath(1000.), 'ldmx-det-v12')

    """
    
    sim = simulator.simulator( "ecal_dark_brem_%sMeV" % str(ap_mass) )
    
    sim.description = "One e- fired far upstream with Dark Brem turned on and biased up in ECal"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    sim.beamSpotSmear = [ 20., 80., 0. ] #mm
    
    # Activiate dark bremming with a certain A' mass and LHE library
    from LDMX.SimCore import dark_brem
    db_model = dark_brem.VertexLibraryModel( lhe )
    db_model.threshold = 2. #GeV - minimum energy electron needs to have to dark brem
    db_model.epsilon   = 0.01 #decrease epsilon from one to help with Geant4 biasing calculations
    sim.dark_brem.activate( ap_mass , db_model )

    import math
    factor = ( sim.dark_brem.ap_mass**math.log10( sim.dark_brem.ap_mass ) ) / ( sim.dark_brem.model.epsilon ** 2 )
    
    # Biasing dark brem up inside of the ecal volumes
    sim.biasingOn()
    sim.biasingConfigure( 'eDarkBrem' , 'ecal' , 0. , factor , 
                allPtl = True, incidentOnly = False)
    
    # the following filters are in a library that needs to be included
    from LDMX.Biasing import include
    include.library()

    sim.actions.extend([ 
            # Abort events if the electron doesn't get to the ECal with 3.5GeV
            filters.PrimaryToEcalFilter( 3500. ),
            # Only keep events when a dark brem happens in the target
            filters.DarkBremFilter.ecal( 2000. , 3 ),
            # Keep all of the dark brem daughters. 
            filters.TrackProcessFilter.dark_brem()
    ])
    
    return sim
