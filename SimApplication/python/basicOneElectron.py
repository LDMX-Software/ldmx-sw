#!/usr/bin/python

from LDMX.Framework import ldmxcfg

basicOneElectron = ldmxcfg.Producer( "basicOneElectron" , "ldmx::Simulator" )

from LDMX.Detectors.makePath import makeDetectorPath

basicOneElectron.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
basicOneElectron.parameters[ "runNumber" ] = 1
basicOneElectron.parameters[ "verbosity" ] = 3
basicOneElectron.parameters[ "description"] = "One 4GeV electron shot from far upstream."

from LDMX.SimApplication import generators

basicOneElectron.parameters[ "generators" ] = [ generators.farUpstreamSingleElectron() ]
