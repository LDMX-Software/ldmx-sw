
####################################################################
# Template for New Dark Brem Signal Generation
# Import with:
#   from LDMX.Biasing.dark_brem import dark_brem
# Use:
#   myDarkBremSim = dark_brem.get( massAPrime , pathToLHEFile )
#   #where massAPrime in MeV

from LDMX.Framework import ldmxcfg
from LDMX.Detector.makePath import makeDetectorPath
from LDMX.SimApplication import generators
from LDMX.Biasing import track_filters, event_filters

def get( massAPrime , pathToLHEFile ) :
    darkBremOn = ldmxcfg.Producer( "darkBrem_" + str(massAPrime) + "_MeV" , "ldmx::Simulator")
    
    darkBremOn.parameters[ "description" ] = "One e- fired far upstream with Dark Brem turned on and biased up in target"
    darkBremOn.parameters[ "detector" ] = makeDetectorPath( "ldmx-det-full-v12-fieldmap-magnet" )
    darkBremOn.parameters[ "generators" ] = [ farUpstreamSingleElectron() ]
    
    # Bias the electron dark brem process inside of the target
    #   These commands allow us to restrict the dark brem process to a given volume.
    darkBremOn.parameters[ "biasing.enabled" ] = True
    darkBremOn.parameters[ "biasing.particle"] = "e-"
    darkBremOn.parameters[ "biasing.process" ] = "eDBrem"
    darkBremOn.parameters[ "biasing.volume"  ] = "target" #options: target, ecal
    darkBremOn.parameters[ "biasing.factor"  ] = 1000000 #this factor is only applied in the volume defined in biasing
    
    darkBremOn.parameters[ "darkbrem.method" ] = 1 #Forward only

    darkBremOn.parameters[ "MassAPrime" ] = massAPrime #MeV
    darkBremOn.parameters[ "darkbrem.madGraphFilePath" ] = pathToLHEFile
    
    # Then give the UserAction to the simulation so that it knows to use it
    darkBremOn.parameters['actions'] = [ 
            event_filters.targetDarkFilter() , #only keep events when a dark brem happens in the target
            track_filters.keepDarkTracks()     #keep all eDBrem children
            ]
    
    return darkBremOn
