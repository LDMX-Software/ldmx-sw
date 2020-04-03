
####################################################################
# Template for studying PN interactions in Target
# Import with:
#   from LDMX.Biasing.target_pn import target_pn
# Use:
#   myTargetPNSim = target_pn.get()

from LDMX.Framework import ldmxcfg
from LDMX.Detector.makePath import * #both detector and scoring planes path
from LDMX.SimApplication import generators
from LDMX.Biasing import event_filters, track_filters

def get( ) :
    
    #
    # Instantiate the simulator.  Before doing this, the shared library containing
    # the simulator needs to be loaded.  This is usually done from the top level
    # configure file.
    #
    simulator = ldmxcfg.Producer("targetPN", "ldmx::Simulator")
    
    #
    # Set the path to the detector to use.
    #
    # The detectors installed with ldmx-sw can be accessed using the makeDetectorPath function.
    # Otherwise, you can provide the full path yourself.
    #
    simulator.parameters["detector"] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
    
    #
    # Set run parameters
    #
    simulator.parameters["runNumber"] = 0
    simulator.parameters["description"] = "ECal photo-nuclear, xsec bias 450"
    simulator.parameters["randomSeeds"] = [ 1, 2 ]
    simulator.parameters["beamspotSmear"] = [20., 80.]
    
    #
    # Fire an electron upstream of the tagger tracker
    #
    # A 4GeV single electron generator is so common that you
    # can pull it in from the generators module.
    #
    simulator.parameters['generators'] = [ generators.farUpstreamSingle4GeVElectron() ]
    
    #
    # Enable the scoring planes 
    #
    # Same comments about path to gdml as for the detectors
    #
    #simulator.parameters["scoringPlanes"] = makeScoringPlanesPath( "ldmx-det-full-v12-fieldmap-magnet" )
    
    #
    # Enable and configure the biasing
    #
    simulator.parameters['biasing.enabled'] = True
    simulator.parameters['biasing.particle'] = 'gamma'
    simulator.parameters['biasing.process'] = 'photonNuclear'
    simulator.parameters['biasing.volume'] = 'target'
    simulator.parameters['biasing.threshold'] = 2500.
    simulator.parameters['biasing.factor'] = 450
    
    #
    # Save tracks of particles created in the photonuclear reaction
    #
    track_process_filter = simcfg.UserAction('trackProcessFilter', 'ldmx::TrackProcessFilter')
    track_process_filter.parameters['process'] = ['photonNuclear']
    
    #
    # Configure the sequence in which user actions should be called.
    #
    simulator.parameters["actions"] = [
            event_filters.targetBremFilter(), #only consider events where a hard brem occurs
            event_filters.targetPNFilter(),   #only consider events where a PN reaction happnes in the target
            track_filters.keepPNTracks()      #keep all PN children
            ]

    return simulator
