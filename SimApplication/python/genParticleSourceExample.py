#!/usr/bin/python

from LDMX.Framework import ldmxcfg

genParticleSourceExample = ldmxcfg.Producer( "genParticleSourceExample", "ldmx::Simulator")

genParticleSourceExample.parameters[ "description" ] = "Example of a General Particle Source"
genParticleSourceExample.parameters[ "postInitCommands" ] = [
        "/ldmx/generators/gps/enable",
        "/gps/particle pi-",
        "/gps/direction 0 0 4.0",
        "/gps/energy 4.0 GeV",
        "/gps/position 0 0 -1 mm"
        ]
