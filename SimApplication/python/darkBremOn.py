#!/usr/bin/python

####################################################################
# Template for New Dark Brem Signal Generation
# Import with:
#   from LDMX.SimApplication.darkBremOn import darkBremOn
# Still Need to Define:
#   1. path to dark brem input LHE file ('darkbrem.madgraphfilepath')
#   2. detector description ('detector')
#   3. mass of A' in MeV ('APrimeMass')

from LDMX.Framework import ldmxcfg
from LDMX.SimApplication import simcfg

darkBremOn = ldmxcfg.Producer( "darkBremOn", "ldmx::Simulator")

darkBremOn.parameters[ "description" ] = "One e- fired far upstream with Dark Brem turned on and biased up in target"

darkBremOn.parameters[ "generators" ] = [ 'gun' ]
darkBremOn.parameters[ "gun.particle" ] = 'e-'
darkBremOn.parameters[ "gun.position" ] = [ -27.926, 5, -700 ] #mm
darkBremOn.parameters[ "gun.time"     ] = 0. #ns
darkBremOn.parameters[ "gun.energy"   ] = 4.0 #GeV
darkBremOn.parameters[ "gun.direction"] = [ 313.8 / 4000.0 , 0, 3987.7 / 4000.0 ] #unit less

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

# Keep A' no matter what
#keepAPrime = simcfg.UserAction("keepAPrime" , "ldmx::TrackFilter" )

# Then give the UserAction to the simulation so that it knows to use it
darkBremOn.parameters['actions'] = [ target_darkbrem_filter ]

