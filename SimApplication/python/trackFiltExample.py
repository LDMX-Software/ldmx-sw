#!/usr/bin/python

from LDMX.Framework import ldmxcfg

trackFiltExample = ldmxcfg.Producer( "trackFiltExample", "ldmx::Simulator")

trackFiltExample.parameters[ "description" ] = "Example on how to run simulation with track filtering plugin"
trackFiltExample.parameters[ "postInitCommands" ] = [
        # basic upstream electron gun
        "/gun/particle e-",
        "/gun/energy 4 GeV",
        "/gun/position -27.926 5 -700 mm",
        "/gun/direction 0.3138 0 3.9877 GeV",
        # Bias PN reactions really high so they actually happen so you can see the track filtering working
        "/ldmx/plugins/load PhotonuclearXsecBiasingPlugin",
        "/ldmx/plugins/PhotonuclearXsecBiasingPlugin/xsecFactor 1000.",
        # enable track filtering
        "/ldmx/plugins/load TrackFilterPlugin",
        "/ldmx/plugins/TrackFilterPlugin/verbose 2",
        # save neutrons above 5 MeV KE from PN interactions
        "/ldmx/plugins/TrackFilterPlugin/threshold 5 MeV",
        "/ldmx/plugins/TrackFilterPlugin/pdgid 2112",
        "/ldmx/plugins/TrackFilterPlugin/process photonNuclear true",
        "/ldmx/plugins/TrackFilterPlugin/CalorimeterRegion true",
        "/ldmx/plugins/TrackFilterPlugin/create KeepNeutrons",
        # save all parents of PN secondaries
        "/ldmx/plugins/TrackFilterPlugin/parent photonNuclear true",
        "/ldmx/plugins/TrackFilterPlugin/CalorimeterRegion true",
        "/ldmx/plugins/TrackFilterPlugin/create PNParentFilter",
        ]
