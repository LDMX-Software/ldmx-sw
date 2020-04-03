###############################################################################
# Templates for studying biased interactions in Target
# Import with:
#   from LDMX.Biasing import target
#
# @author Tom Eichlersmith, University of Minnesota
###############################################################################

from LDMX.Framework import ldmxcfg
from LDMX.Detector.makePath import * #both detector and scoring planes path
from LDMX.SimApplication import generators
from LDMX.Biasing import event_filters, track_filters

###############################################################################
# @func electronNuclear
# Biasing EN interactions in the target and use a basic one 4GeV electron
#   generator
# User:
#   myTargetENSim = target.electronNuclear()
###############################################################################
def electronNuclear( ) :
    
    #
    # Instantiate the simulator.  Before doing this, the shared library containing
    # the simulator needs to be loaded.  This is usually done from the top level
    # configure file.
    #
    simulator = ldmxcfg.Producer("target_electronNuclear", "ldmx::Simulator")
    
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
    # Configure the sequence in which user actions should be called.
    #
    simulator.parameters["actions"] = [
            event_filters.targetENFilter(), #only consider events where an EN interaction happens in the target
            track_filters.keepENTracks()    #keep all EN children
            ]

    return simulator

###############################################################################
# @func photonNuclear
# Biasing PN interactions in the target and use a basic one 4GeV electron
#   generator
# User:
#   myTargetPNSim = target.photonNuclear()
###############################################################################
def photonNuclear( ) :
    
    #
    # Instantiate the simulator.  Before doing this, the shared library containing
    # the simulator needs to be loaded.  This is usually done from the top level
    # configure file.
    #
    simulator = ldmxcfg.Producer("target_photonNuclear", "ldmx::Simulator")
    
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

###############################################################################
# @func darkBrem
# Biasing dark brem interactions in the target and use a basic one 4GeV electron
#   generator
# User:
#       myTargetDarkBremSim = target.darkBrem( massAPrime , lheFile )
#   where massAPrime is the mass of the A' in MeV and lheFile is the path
#   to the LHE file to use as vertices of the dark brem
###############################################################################
def darkBrem( massAPrime , lheFile )
    darkBremOn = ldmxcfg.Producer( "darkBrem_" + str(massAPrime) + "_MeV" , "ldmx::Simulator")
    
    darkBremOn.parameters[ "description" ] = "One e- fired far upstream with Dark Brem turned on and biased up in target"
    darkBremOn.parameters[ "detector" ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
    darkBremOn.parameters[ "generators" ] = [ generators.farUpstreamSingle4GeVElectron() ]
    
    # Bias the electron dark brem process inside of the target
    #   These commands allow us to restrict the dark brem process to a given volume.
    darkBremOn.parameters[ "biasing.enabled" ] = True
    darkBremOn.parameters[ "biasing.particle"] = "e-"
    darkBremOn.parameters[ "biasing.process" ] = "eDBrem"
    darkBremOn.parameters[ "biasing.volume"  ] = "target" #options: target, ecal
    darkBremOn.parameters[ "biasing.factor"  ] = 1000000 #this factor is only applied in the volume defined in biasing
    
    darkBremOn.parameters[ "darkbrem.method" ] = 1 #Forward only

    darkBremOn.parameters[ "MassAPrime" ] = massAPrime #MeV
    darkBremOn.parameters[ "darkbrem.madGraphFilePath" ] = lheFile
    
    # Then give the UserAction to the simulation so that it knows to use it
    darkBremOn.parameters['actions'] = [ 
            event_filters.targetDarkFilter() , #only keep events when a dark brem happens in the target
            track_filters.keepDarkTracks()     #keep all eDBrem children
            ]
    
    return darkBremOn
