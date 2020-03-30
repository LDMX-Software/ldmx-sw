from LDMX.Framework import ldmxcfg
from LDMX.SimApplication import simcfg

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
simulator.parameters["detector"] = makeDetectorPath( "ldmx-det-full-v9-fieldmap-magnet" )


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
from LDMX.SimApplication import generators
simulator.parameters['generators'] = [ generators.farUpstreamSingle4GeVElectron() ]

#
# Enable the scoring planes 
#
# Same comments about path to gdml as for the detectors
#
#simulator.parameters["scoringPlanes"] = "detectors/scoring_planes/detector.gdml"

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
# Only consider events with a hard brem
# 
target_brem_filter = simcfg.UserAction("targetBrem", "ldmx::TargetBremFilter")
target_brem_filter.parameters['volume'] = 'target_PV'
target_brem_filter.parameters['recoilEnergyThreshold'] = 1500.
target_brem_filter.parameters['bremEnergyThreshold'] = 2500.

#
# Only consider events where a photonuclear reaction happens in the target 
#
ecal_process_filter = simcfg.UserAction("ecalProcess", "ldmx::EcalProcessFilter")
ecal_process_filter.parameters['process'] = 'photonNuclear'
ecal_process_filter.parameters['volume'] = 'ecal'

#
# Save tracks of particles created in the photonuclear reaction
#
track_process_filter = simcfg.UserAction('trackProcessFilter', 'ldmx::TrackProcessFilter')
track_process_filter.parameters['process'] = ['photonNuclear']

#
# Configure the sequence in which user actions should be called.
#
simulator.parameters["actions"] = [target_brem_filter, ecal_process_filter, track_process_filter]
