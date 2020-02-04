#!/usr/bin/python

from LDMX.Framework import ldmxcfg

basicOneElectron = ldmxcfg.Producer( "basicOneElectron", "ldmx::Simulator")

basicOneElectron.parameters[ "description" ] = "One e- fired far upstream without any plugins or modifications to the physics"
basicOneElectron.parameters[ "mpgNparticles" ] = 1
basicOneElectron.parameters[ "mpgPdgId"      ] = 11
basicOneElectron.parameters[ "mpgVertex"     ] = [ -27.926, 5, -700 ]
basicOneElectron.parameters[ "mpgMomentum"   ] = [ 313.8, 0, 3987.7 ]
