#!/usr/bin/python

from LDMX.Framework import ldmxcfg

targetENBias = ldmxcfg.Producer( "targetENBias", "ldmx::Simulator")

targetENBias.parameters[ "description" ] = "Sample of events where 4GeV undergoes EN reaction in target"
targetENBias.parameters[ "preInitCommands" ] = [
        "/ldmx/biasing/enable",
        "/ldmx/biasing/particle e-",
        "/ldmx/biasing/process electronNuclear", 
        "/ldmx/biasing/volume target",
        "/ldmx/biasing/threshold 0"
        ]
targetENBias.parameters[ "postInitCommands" ] = [
        "/ldmx/biasing/xsec/bias_incident", #only bias incident particle
        "/ldmx/biasing/xsec/particle e-",
        "/ldmx/biasing/xsec/process electronNuclear",
        "/ldmx/biasing/xsec/threshold 0",
        "/ldmx/biasing/xsec/factor 100000",
        "/gun/particle e-",
        "/gun/energy 4 GeV",
        "/gun/position -27.926 5 -700 mm",
        "/gun/direction 0.3138 0 3.9877 GeV",
        "/ldmx/plugins/load TargetProcessFilter libBiasing.so" # filters for process specified in target in biasing
        ]
