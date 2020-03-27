#!/usr/bin/python

from LDMX.Framework import ldmxcfg

basicOneElectron = ldmxcfg.Producer( "basicOneElectron" , "ldmx::Simulator" )

from LDMX.Detectors.makeDetectorPath import makeDetectorPath

basicOneElectron.parameters[ "detector"  ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
basicOneElectron.parameters[ "runNumber" ] = 1
basicOneElectron.parameters[ "verbosity" ] = 3
basicOneElectron.parameters[ "description"] = "One 4GeV electron shot from far upstream."

from LDMX.SimApplication import simcfg

farUpstreamElectron = simcfg.PrimaryGenerator( 'farUpstreamGenerator' , 'ldmx::ParticleGun' )
farUpstreamElectron.parameters[ 'particle'  ] = 'e-'
farUpstreamElectron.parameters[ 'position'  ] = [ -27.926, 5, -700 ] #mm
farUpstreamElectron.parameters[ 'direction' ] = [ 313.8 / 4000 , 0, 3987.7/4000 ] #unitless
farUpstreamElectron.parameters[ 'energy'    ] = 4.0 #GeV

basicOneElectron.parameters[ "generators" ] = [ farUpstreamElectron ]
