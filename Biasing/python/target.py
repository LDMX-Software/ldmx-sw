"""Example configurations for producing biased interactions in the target. 

    Example
    -------
        
        from LDMX.Biasing import target
"""

from LDMX.SimCore import generators
from LDMX.SimCore import simulator
from LDMX.Biasing import filters
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
    sim.biasingOn()
    sim.biasingConfigure( 'electronNuclear' , 'target' , 0. , int(1e8) )

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            filters.TargetENFilter(2500.),
            filters.TrackProcessFilter.electro_nuclear()      
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
    sim.biasingOn()
    sim.biasingConfigure(
            'photonNuclear' #process
            , 'target' #volume
            , 2500. #threshold in MeV
            , 450 #factor
            )
   
    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(),
            filters.TargetPNFilter(),   
            # Tag all photo-nuclear tracks to persist them to the event.
            filters.TrackProcessFilter.photo_nuclear()
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
        The path to the LHE file to use as vertices of the dark brem. 
    detector : str
        Name of detector to simulate in

    Return
    ------
    Instance of the sim configured for dark-brem production in the target.

    Example
    -------

        target_ap_sim = target.dark_brem(1000, 'path/to/lhe', 'ldmx-det-v12')


    """
    sim = simulator.simulator( "darkBrem_" + str(massAPrime) + "_MeV" )
    
    sim.description = "One e- fired far upstream with Dark Brem turned on and biased up in target"
    sim.setDetector( detector , True )
    sim.generators.append( generators.single_4gev_e_upstream_tagger() )
    
    # Bias the electron dark brem process inside of the target
    # These commands allow us to restrict the dark brem process to a given 
    # volume.
    sim.biasingOn()
    sim.biasingConfigure(
            'eDBrem' #process
            , 'target' #volume
            , 0. #threshold
            , 1000000 #factor
            )
    
    sim.darkBremOn(
            massAPrime #MeV
            , lheFile
            , 1 #Forward Only
            )
    
    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Then give the UserAction to the simulation so that it knows to use it
    sim.actions.extend([ 
            filters.DarkBremFilter(), 
            filters.TrackProcessFilter.dark_brem()     
    ])
    
    return sim
