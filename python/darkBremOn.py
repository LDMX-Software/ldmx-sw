#!/usr/bin/python

from LDMX.Framework import ldmxcfg

darkBremOn = ldmxcfg.Producer( "darkBremOn", "ldmx::Simulator")

darkBremOn.parameters[ "description" ] = "One e- fired far upstream with Dark Brem turned on and biased up in target"
darkBremOn.parameters[ "mpgNparticles" ] = 1
darkBremOn.parameters[ "mpgPdgId"      ] = 11
darkBremOn.parameters[ "mpgVertex"     ] = [ -27.926, 5, -700 ]
darkBremOn.parameters[ "mpgMomentum"   ] = [ 313.8, 0, 3987.7 ]
darkBremOn.parameters[ "APrimeMass"    ] = 10. #MeV
darkBremOn.parameters[ "MadGraphFilePath" ] = "mad_graph_data.lhe"

darkBremOn.parameters[ "preInitCommands" ] = [
        # Bias the electron dark brem process inside of the target
        #   These commands allow us to restrict the dark brem process to a given volume.
        #   The actual biasing factor is passed after run initialization
        "/ldmx/biasing/enable",
        "/ldmx/biasing/particle e-",
        "/ldmx/biasing/process eDBrem",
        "/ldmx/biasing/volume target" #options: target, ecal
        ]

darkBremOn.parameters[ "postInitCommands" ] = [
        # Set the biasing cross section factor
        #   The factor passed here is applied only in the volume defined above
        "/ldmx/biasing/xsec/particle e-",
        "/ldmx/biasing/xsec/factor 1000000", #this factor is only applied in the volume defined pre-initialize
        # DarkBremFilter checks that a dark brem happened in a given volume
        #   aborts the event if it didn't happen
        #   can generate A' without this, but helpful for generating pure signal sample
        "/ldmx/plugins/load DarkBremFilter libBiasing.so",
        "/ldmx/plugins/DarkBremFilter/volume target", #should match volume defined for biasing
        "/ldmx/plugins/DarkBremFilter/verbose 2",
        # Save A' no matter what
        #   The hard-coded PDG ID for the A' is defined in SimCore/src/G4APrime.cxx
        #   Not necessary if A' is generated outside calorimeter region, but good to have just in case
        "/ldmx/plugins/load TrackFilterPlugin",
        "/ldmx/plugins/TrackFilterPlugin/pdgid 622",
        "/ldmx/plugins/TrackFilterPlugin/create KeepAPrime",
        ]
