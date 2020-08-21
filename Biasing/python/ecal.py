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
    sim.randomSeeds = [ 1, 2 ]
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasingOn()
    sim.biasingConfigure( 'photonNuclear' , 'ecal' , 2500. , 450 )

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

        ecal_ap_sim = ecal.dark_brem(1000, 'path/to/lhe', 'ldmx-det-v12')


    """
    
    sim = simulator.simulator( "darkBrem_%sMeV" % str(massAPrime) )
    
    sim.description = "One e- fired far upstream with Dark Brem turned on and biased up in ECal"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    
    # Bias the electron dark brem process inside of the ECal
    # These commands allow us to restrict the dark brem process to a given 
    # volume.
    sim.biasingOn()
    sim.biasingConfigure( 'eDBrem' , 'ecal' , 0. , 100000 )
    
    sim.darkBremOn( massAPrime #MeV
            , lheFile 
            , 1 ) #Forward Only
    
    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Then give the UserAction to the simulation so that it knows to use it
    sim.actions().extend([ 
            # Only keep events when a dark brem happens in the target
            filters.DarkBremFilter('ecal') , 
            # Keep all of the dark brem daughters. 
            filters.TrackProcessFilter.dark_brem()
    ])
    
    return sim
