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
    sim.description = "ECal photo-nuclear, xsec bias 450"
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('ecal',450.,2500.,only_children_of_primary = True) ]

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

def nonfiducial_photo_nuclear( detector, generator ) :

    # Instantiate the simulator. 
    sim = simulator.simulator("photo-nuclear")
    
    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.setDetector( detector , True )
    
    # Set run parameters
    sim.description = "ECal photo-nuclear, xsec bias 450"
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('ecal',450.,2500.,only_children_of_primary = True) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(),
            # Only considers events that are Non-Fiducial (Doesn't enter an ECal volume)
            filters.NonFiducialFilter(),
            # Only consider events where a PN reaction happens in the ECal
            filters.EcalProcessFilter(),     
            # Tag all photo-nuclear tracks to persist them to the event.
            util.TrackProcessFilter.photo_nuclear()
    ])

    return sim

def gamma_mumu(detector, generator) :
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
    sim = simulator.simulator("ecal_gammamumu")

    # Set the path to the detector to use
    # Also tell the simulator to include scoring planes
    sim.setDetector( detector, True )

    # Set run parameters
    sim.description = "gamma --> mu+ mu-, xsec bias 3e4"
    sim.beamSpotSmear = [20., 80., 0.]

    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.GammaToMuPair('ecal', 3.E4, 2500.) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called
    sim.actions.extend([
            filters.TaggerVetoFilter(),
            filters.TargetBremFilter(),
            filters.EcalProcessFilter(process='GammaToMuPair'),
            util.TrackProcessFilter.gamma_mumu()
    ])

    return sim
