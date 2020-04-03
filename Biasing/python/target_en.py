
####################################################################
# Template for studying EN interactions in Target
# Import with:
#   from LDMX.Biasing.target_en import target_en
# Use:
#   myTargetENSim = target_en.get()

from LDMX.Framework import ldmxcfg
from LDMX.Detector.makePath import * #both detector and scoring planes path
from LDMX.SimApplication import generators
from LDMX.SimApplication import simcfg

def get( ) :
    
    #
    # Instantiate the simulator.  Before doing this, the shared library containing
    # the simulator needs to be loaded.  This is usually done from the top level
    # configure file.
    #
    simulator = ldmxcfg.Producer("simulator", "ldmx::Simulator")
    
    #
    # Set the path to the detector to use.
    #
    # The detectors installed with ldmx-sw can be accessed using the makeDetectorPath function.
    # Otherwise, you can provide the full path yourself.
    #
    from LDMX.Detectors.makePath import *
    simulator.parameters["detector"] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
    
    #
    # Set run parameters
    #
    simulator.parameters["runNumber"] = 0
    simulator.parameters["description"] = "Target electron-nuclear, xsec bias 1e8"
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
    simulator.parameters['biasing.particle'] = 'e-'
    simulator.parameters['biasing.process'] = 'electronNuclear'
    simulator.parameters['biasing.volume'] = 'target'
    simulator.parameters['biasing.factor'] = 1e8
    
    #
    # Only consider events where a electronNuclear interaction happens in the target
    #
    target_process_filter = simcfg.UserAction("targetProcess", "ldmx::TargetENProcessFilter")
    target_process_filter.parameters['process'] = 'electronNuclear'
    target_process_filter.parameters['volume'] = 'target'
    target_process_filter.parameters['recoilThreshold'] = 4000 #MeV
    
    #
    # Save tracks of particles created in the electronNuclear reaction
    #
    track_process_filter = simcfg.UserAction('trackProcessFilter', 'ldmx::TrackProcessFilter')
    track_process_filter.parameters['process'] = ['electronNuclear']
    
    #
    # Configure the sequence in which user actions should be called.
    #
    simulator.parameters["actions"] = [target_process_filter, track_process_filter]

    return simulator
