
####################################################################
# Template for studying PN interactions in ECal
# Import with:
#   from LDMX.Biasing.ecal_pn import ecal_pn
# Use:
#   myEcalPNSim = ecal_pn.get()

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
    simulator = ldmxcfg.Producer("ecal_pn", "ldmx::Simulator")
    
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
    simulator.parameters['biasing.volume'] = 'ecal'
    simulator.parameters['biasing.threshold'] = 2500.
    simulator.parameters['biasing.factor'] = 450
    
    #
    # Configure the sequence in which user actions should be called.
    #   the order is important, it is the order they will be called in, so put event filters first
    #
    simulator.parameters["actions"] = [
            event_filters.targetBremFilter(), #only consider events with a hard brem
            event_filters.ecalPNFilter()    , #only consider events with a PN reaction in the ECal
            track_filters.keepPNTracks()      #make sure to keep tracks created by PN reaction
            ]

    return simulator
