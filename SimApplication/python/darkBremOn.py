#!/usr/bin/python

####################################################################
# Template for New Dark Brem Signal Generation
# Import with:
#   from LDMX.SimApplication.darkBremOn import darkBremOn
# Still Need to Define:
#   1. path to dark brem input LHE file ('darkbrem.madgraphfilepath')
#   2. mass of A' in MeV ('APrimeMass')

from LDMX.Framework import ldmxcfg

darkBremOn = ldmxcfg.Producer( "darkBremOn", "ldmx::Simulator")

darkBremOn.parameters[ "description" ] = "One e- fired far upstream with Dark Brem turned on and biased up in target"

from LDMX.Detector.makePath import makeDetectorPath
darkBremOn.parameters[ "detector" ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )

from LDMX.SimApplication import generators
darkBremOn.parameters[ "generators" ] = [ farUpstreamSingleElectron() ]

# Bias the electron dark brem process inside of the target
#   These commands allow us to restrict the dark brem process to a given volume.
darkBremOn.parameters[ "biasing.enabled" ] = True
darkBremOn.parameters[ "biasing.particle"] = "e-"
darkBremOn.parameters[ "biasing.process" ] = "eDBrem"
darkBremOn.parameters[ "biasing.volume"  ] = "target" #options: target, ecal
darkBremOn.parameters[ "biasing.factor"  ] = 1000000 #this factor is only applied in the volume defined in biasing

darkBremOn.parameters[ "darkbrem.method" ] = 1 #Forward only

# DarkBremFilter checks that a dark brem happened in a given volume
#   aborts the event if it didn't happen
#   can generate A' without this, but helpful for generating pure signal sample
target_darkbrem_filter = simcfg.UserAction("targetDarkBrem", "ldmx::DarkBremFilter")
target_darkbrem_filter.parameters['volume'] = 'target_PV' # volume to look inside of
target_darkbrem_filter.parameters['verbosity'] = 2

# Keep Dark Brem children no matter what
keepDarkBremParticles = simcfg.UserAction("keepDarkBremParticles" , "ldmx::TrackProcessFilter" )
keepDarkBremParticles.parameters[ "process" ] = [ "eDBrem" ]

# Then give the UserAction to the simulation so that it knows to use it
darkBremOn.parameters['actions'] = [ target_darkbrem_filter , keepDarkBremParticles ]

