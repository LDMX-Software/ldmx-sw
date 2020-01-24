#!/usr/bin/python

from LDMX.Framework import ldmxcfg

basicOneElectron = ldmxcfg.Producer( "basicOneElectron", "ldmx::Simulator")

basicOneElectron.parameters[ "description" ] = "One e- fired far upstream without any plugins or modifications to the physics"
basicOneElectron.parameters[ "postInitCommands" ] = [
        "/gun/particle e-",
        "/gun/energy 4 GeV",
        "/gun/position -27.926 5 -700 mm",
        "/gun/direction 0.3138 0 3.9877 GeV"
        ]
