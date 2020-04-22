""" Example configurations for producing biased interactions in the target. 

    Example
    -------
        
        from LDMX.Biasing import target
"""

from LDMX.Framework import ldmxcfg
from LDMX.Detectors.makePath import * 
from LDMX.SimApplication import generators
from LDMX.Biasing import event_filters, track_filters

def electro_nuclear( detector ) :
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
    Instance of the simulator configured for target electro-nuclear.

    Example
    -------

        target_en_sim = target.electro_nuclear('ldmx-det-v12')

    """
    
    # Instantiate the simulator.  Before doing this, the shared library containing
    # the simulator needs to be loaded.  This is usually done from the top level
    # configure file.
    simulator = ldmxcfg.Producer("target_electronNuclear", "ldmx::Simulator")
    
    # Set the path to the detector to use.
    #
    # The detectors installed with ldmx-sw can be accessed using the makeDetectorPath function.
    # Otherwise, you can provide the full path yourself.
    simulator.parameters["detector"] = makeDetectorPath( detector )
    
    # Set run parameters
    simulator.parameters["runNumber"] = 0
    simulator.parameters["description"] = "Target electron-nuclear, xsec bias 1e8"
    simulator.parameters["randomSeeds"] = [ 1, 2 ]
    simulator.parameters["beamSpotSmear"] = [20., 80., 0.]
    
    # Fire an electron upstream of the tagger tracker
    #
    # A 4GeV single electron generator is so common that you
    # can pull it in from the generators module.
    simulator.parameters['generators'] = [ generators.farUpstreamSingle4GeVElectron() ]
    
    # Enable the scoring planes 
    simulator.parameters["scoringPlanes"] = makeScoringPlanesPath( detector )
    
    # Enable and configure the biasing
    simulator.parameters['biasing.enabled'] = True
    simulator.parameters['biasing.particle'] = 'e-'
    simulator.parameters['biasing.process'] = 'electronNuclear'
    simulator.parameters['biasing.volume'] = 'target'
    simulator.parameters['biasing.factor'] = 1e8

    tagger_veto_filter = simcfg.UserAction("tagger_veto_filter", "ldmx::TaggerVetoFilter")
    tagger_veto_filter.parameters['threshold'] = 3800.
   
    # Save tracks of particles created in the photo-nuclear reaction
    track_process_filter = simcfg.UserAction('trackProcessFilter', 'ldmx::TrackProcessFilter')
    track_process_filter.parameters['process'] = 'electronNuclear'

    # Configure the sequence in which user actions should be called.
    simulator.parameters["actions"] = [
            tagger_veto_filter,
            event_filters.targetENFilter(), #only consider events where an EN interaction happens in the target
            # Tag all electro-nuclear tracks to persist them to the event.
            track_process_filter        
    ]

    return simulator

def photo_nuclear( detector ) :
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
    Instance of the simulator configured for target photo-nuclear.

    Example
    -------

        target_pn_sim = target.photo_nuclear('ldmx-det-v12')

    """


    # Instantiate the simulator.  Before doing this, the shared library containing
    # the simulator needs to be loaded.  This is usually done from the top level
    # configure file.
    simulator = ldmxcfg.Producer("target_photonNuclear", "ldmx::Simulator")
    
    # Set the path to the detector to use.
    #
    # The detectors installed with ldmx-sw can be accessed using the makeDetectorPath function.
    # Otherwise, you can provide the full path yourself.
    simulator.parameters["detector"] = makeDetectorPath( detector )
    
    # Set run parameters
    simulator.parameters["runNumber"] = 0
    simulator.parameters["description"] = "ECal photo-nuclear, xsec bias 450"
    simulator.parameters["randomSeeds"] = [ 1, 2 ]
    simulator.parameters["beamSpotSmear"] = [20., 80., 0.]
    
    # Fire an electron upstream of the tagger tracker
    #
    # A 4GeV single electron generator is so common that you
    # can pull it in from the generators module.
    simulator.parameters['generators'] = [ generators.farUpstreamSingle4GeVElectron() ]
    
    # Enable the scoring planes 
    #
    # Same comments about path to gdml as for the detectors
    simulator.parameters["scoringPlanes"] = makeScoringPlanesPath( detector )
    
    # Enable and configure the biasing
    simulator.parameters['biasing.enabled'] = True
    simulator.parameters['biasing.particle'] = 'gamma'
    simulator.parameters['biasing.process'] = 'photonNuclear'
    simulator.parameters['biasing.volume'] = 'target'
    simulator.parameters['biasing.threshold'] = 2500.
    simulator.parameters['biasing.factor'] = 450
   
    tagger_veto_filter = simcfg.UserAction("tagger_veto_filter", "ldmx::TaggerVetoFilter")
    tagger_veto_filter.parameters['threshold'] = 3800.

    # Save tracks of particles created in the photo-nuclear reaction
    track_process_filter = simcfg.UserAction('trackProcessFilter', 'ldmx::TrackProcessFilter')
    track_process_filter.parameters['process'] = 'photonNuclear'
    
    # Configure the sequence in which user actions should be called.
    simulator.parameters["actions"] = [
            tagger_veto_filter, 
            event_filters.targetBremFilter(), 
            event_filters.targetPNFilter(),   
            # Tag all photo-nuclear tracks to persist them to the event.
            track_process_filter        
    ]

    return simulator

def dark_brem( ap_mass , lhe, detector ) :
    """Example configuration for producing dark brem interactions in the target. 

    This configures the simulator to fire a 4 GeV electron upstream of the 
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
        Path to the detector.

    Return
    ------
    Instance of the simulator configured for dark-brem production in the target.

    Example
    -------

        target_ap_sim = target.dark_brem(1000, 'path/to/lhe', 'ldmx-det-v12')


    """
    simulator = ldmxcfg.Producer( "darkBrem_" + str(massAPrime) + "_MeV" , "ldmx::Simulator")
    
    simulator.parameters[ "description" ] = "One e- fired far upstream with Dark Brem turned on and biased up in target"
    simulator.parameters[ "detector" ] = makeDetectorPath( detector )
    simulator.parameters[ "scoringPlanes" ] = makeScoringPlanesPath( detector )
    simulator.parameters[ "generators" ] = [ generators.farUpstreamSingle4GeVElectron() ]
    
    # Bias the electron dark brem process inside of the target
    # These commands allow us to restrict the dark brem process to a given 
    # volume.
    simulator.parameters[ "biasing.enabled" ] = True
    simulator.parameters[ "biasing.particle"] = "e-"
    simulator.parameters[ "biasing.process" ] = "eDBrem"
    # Options: target, ECal
    simulator.parameters[ "biasing.volume"  ] = "target"
    simulator.parameters[ "biasing.factor"  ] = 1000000 
    
    simulator.parameters[ "darkbrem.method" ] = 1 # Forward only

    simulator.parameters[ "APrimeMass" ] = massAPrime #MeV
    simulator.parameters[ "darkbrem.madgraphfilepath" ] = lheFile
    
    # Then give the UserAction to the simulation so that it knows to use it
    simulator.parameters['actions'] = [ 
            event_filters.targetDarkFilter() , 
            track_filters.keepDarkTracks()     
            ]
    
    return simulator
