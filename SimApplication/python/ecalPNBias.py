#!/usr/bin/python

from LDMX.Framework import ldmxcfg

ecalPNBias = ldmxcfg.Producer( "ecalPNBias", "ldmx::Simulator")

ecalPNBias.parameters[ "description" ] = "Sample of events where 4GeV undergoes hard brem in target and PN in ecal"
ecalPNBias.parameters[ "preInitCommands" ] = [
        "/ldmx/biasing/enable",
        "/ldmx/biasing/particle gamma",
        "/ldmx/biasing/process photonNuclear", 
        "/ldmx/biasing/volume ecal",
        "/ldmx/biasing/xsec 1000",
        "/ldmx/biasing/threshold 2500"
        ]
ecalPNBias.parameters[ "postInitCommands" ] = [
        # skip events that don't have a hard brem in the target
        #   sets minimum recoil energy at 1500MeV and brem energy at 2500MeV
        "/ldmx/plugins/load TargetBremFilter libBiasing.so",
        "/ldmx/plugins/TargetBremFilter/volume target_PV",
        "/ldmx/plugins/TargetBremFilter/recoil_threshold 1500",
        "/ldmx/plugins/TargetBremFilter/brem_threshold 2500",
        # filters for PN processes in ecal
        "/ldmx/plugins/load EcalProcessFilter libBiasing.so" 
        "/ldmx/plugins/EcalProcessFilter/volume ecal",
        # normal upstream 4GeV electron gun
        "/gun/particle e-",
        "/gun/energy 4 GeV",
        "/gun/position -27.926 5 -700 mm",
        "/gun/direction 0.3138 0 3.9877 GeV",
        ]
