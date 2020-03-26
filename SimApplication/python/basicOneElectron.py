#!/usr/bin/python

from LDMX.Framework import ldmxcfg

basicOneElectron = ldmxcfg.Producer( "basicOneElectron" , "ldmx::Simulator" )

from LDMX.Detectors.makeDetectorPath import makeDetectorPath

basicOneElectron.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
basicOneElectron.parameters[ "runNumber" ] = 1
basicOneElectron.parameters[ "verbosity" ] = 3
basicOneElectron.parameters[ "description"] = "One 4GeV electron shot from far upstream."
basicOneElectron.parameters[ "generators" ] = [ 'gun' ]
basicOneElectron.parameters[ 'gun.particle' ] = 'e-'
basicOneElectron.parameters[ 'gun.position' ] = [ -27.926, 5, -700 ] #mm
basicOneElectron.parameters[ 'gun.direction'] = [ 313.8 / 4000 , 0, 3987.7/4000 ] #unitless
basicOneElectron.parameters[ 'gun.energy'   ] = 4.0 #GeV
