#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalMuPair = ldmxcfg.Producer( "ecalMuPair", "ldmx::Simulator")

ecalMuPair.parameters[ "description" ] = "Generation of gamma -> 2mu pairs within ECal"
ecalMuPair.parameters[ "preInitCommands" ] = [
        "/ldmx/biasing/enable",
        "/ldmx/biasing/particle gamma",
        "/ldmx/biasing/process GammaToMuPair", #make gamma -> 2mu pretty much certain in ECal
        "/ldmx/biasing/volume Ecal",
        "/ldmx/biasing/xsec 10000000",
        "/ldmx/biasing/threshold 0"
        ]
ecalMuPair.parameters[ "postInitCommands" ] = [
        "/gun/particle e-",
        "/gun/energy 4 GeV",
        "/gun/position -27.926 5 -700 mm",
        "/gun/direction 0.3138 0 3.9877 GeV",
        "/ldmx/plugins/TargetBremFilter libBiasing.so", #filters for hard brem
        "/ldmx/plugins/load EcalProcessFilter libBiasing.so" # filters for process specified in ECal in biasing
        ]
