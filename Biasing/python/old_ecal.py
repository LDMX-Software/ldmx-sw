"""Example configurations for producing biased interactions in the ECal. 

    Example
    -------
        
        from LDMX.Biasing import old_ecal
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

        ecal_pn_sim = old_ecal.photo_nuclear('ldmx-det-v12')

    """


    # Instantiate the simulator. 
    sim = simulator.simulator("photo-nuclear")
    
    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.setDetector( detector , True )

    # Set run parameters
    if generator.energy == 8.0:
          xsec_bias = 550.
          xsec_bias_threshold = 5000.
          tagger_threshold = 7600.
          recoil_max_p = 3000.
          brem_min_e = 5000. 
    else:
          xsec_bias = 450.
          xsec_bias_threshold = 2500.
          tagger_threshold = 3800.
          recoil_max_p = 1500. 
          brem_min_e = 2500.

    sim.description = "ECal photo-nuclear, xsec bias " + str(xsec_bias) + " xsec threshold " + str(xsec_bias_threshold) + " GeV"
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('old_ecal',xsec_bias,xsec_bias_threshold,only_children_of_primary = True) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(recoil_max_p = recoil_max_p, brem_min_e = brem_min_e),
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
    if generator.energy == 8.0:
          xsec_bias = 550.
          xsec_bias_threshold = 5000.
          tagger_threshold = 7600.
          recoil_max_p = 3000.
          brem_min_e = 5000.
    else:
          xsec_bias = 450.
          xsec_bias_threshold = 2500.
          tagger_threshold = 3800.
          recoil_max_p = 1500.
          brem_min_e = 2500.

    sim.description = "Non-fiducial ECal photo-nuclear, xsec bias " + str(xsec_bias) + " xsec threshold " + str(xsec_bias_threshold) + " GeV" 
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('old_ecal',xsec_bias,xsec_bias_threshold,only_children_of_primary = True) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            # Only consider events where a hard brem occurs
            filters.TargetBremFilter(recoil_max_p = recoil_max_p, brem_min_e = brem_min_e),
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

        ecal_mumu_sim = old_ecal.gamma_mumu('ldmx-det-v12')

    """

    # Initiate the sim
    sim = simulator.simulator("ecal_gammamumu")

    # Set the path to the detector to use
    # Also tell the simulator to include scoring planes
    sim.setDetector( detector, True )

    # Set run parameters
    if generator.energy == 8.0:
          xsec_bias = 1.E5
          xsec_bias_threshold = 5000.
          tagger_threshold = 7600.
          recoil_max_p = 3000.
          brem_min_e = 5000.
    else:
          xsec_bias = 3.E4
          xsec_bias_threshold = 2500.
          tagger_threshold = 3800.
          recoil_max_p = 1500.
          brem_min_e = 2500.

    sim.description = "gamma --> mu+ mu-, xsec bias " + str(xsec_bias) + " xsec threshold " + str(xsec_bias_threshold) + " GeV"
    sim.generators.append(generator)

    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.GammaToMuPair('old_ecal', xsec_bias, xsec_bias_threshold) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called
    sim.actions.extend([
            filters.TaggerVetoFilter(thresh = tagger_threshold),
            filters.TargetBremFilter(recoil_max_p = recoil_max_p, brem_min_e = brem_min_e),
            filters.EcalProcessFilter(process='GammaToMuPair'),
            util.TrackProcessFilter.gamma_mumu()
    ])

    return sim

def deep_photo_nuclear( detector, generator, bias_threshold, processes, ecal_min_Z, require_photon_fromTarget = False) :

    # Instantiate the simulator. 
    sim = simulator.simulator("photo-nuclear")
    
    # Set the path to the detector to use.
    #   the second parameter says we want to include scoring planes
    sim.setDetector( detector , True )
    
    # Set run parameters
    # Set run parameters
    if generator.energy == 8.0:
          xsec_bias = 550.
          xsec_bias_threshold = 5000.
    else:
          xsec_bias = 450.
          xsec_bias_threshold = 2500.

    sim.description = "deep ECal photo-nuclear, xsec bias " + str(xsec_bias) + " xsec threshold " + str(xsec_bias_threshold) + " GeV" 
    sim.beamSpotSmear = [20., 80., 0.] #mm
    
    sim.generators.append( generator )
    
    # Enable and configure the biasing
    sim.biasing_operators = [ bias_operators.PhotoNuclear('old_ecal',xsec_bias, xsec_bias_threshold, only_children_of_primary = True, down_bias_conv = False) ]

    # the following filters are in a library that needs to be included
    includeBiasing.library()

    # Configure the sequence in which user actions should be called.
    sim.actions.extend([
            #Make sure all particles above a given threshold are processed first
            util.PartialEnergySorter(0.8*bias_threshold),
            # Make sure the primary electron with a given energy reaches the ECAL
            filters.PrimaryToEcalFilter(0.2*generator.energy*1000),
            # Only considers events when the interaction happend deep in the ECAL
            filters.DeepEcalProcessFilter(bias_threshold, processes, ecal_min_Z, require_photon_fromTarget),
            # Keep intersting particles in the output
            util.TrackProcessFilter.conversion()
    ])

    return sim
