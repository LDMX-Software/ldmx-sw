"""Example configurations for producing biased interactions in the ECal. 

    Example
    -------
        
        from LDMX.Biasing import ecal
"""

from LDMX.SimCore import simulator
from LDMX.SimCore import generators
from LDMX.SimCore import bias_operators
from LDMX.Biasing import filters
from LDMX.Biasing import util
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
    #   In order to conform to older samples,
    #   we can only attach to some of the volumes in the ecal
    #   so we ask for this operator to be attached to the 'old_ecal'
    sim.biasing_operators = [ bias_operators.PhotoNuclear('old_ecal',450.,2500.,only_children_of_primary = True) ]

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
            util.TrackProcessFilter.photo_nuclear()
    ])

    return sim
