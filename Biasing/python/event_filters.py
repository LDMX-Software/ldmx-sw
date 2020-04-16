###############################################################################
# @file event_filters.py
# These UserActions are used to filter events during the simulation
#   i.e. They abort events early not matching certain criteria
#       in order to save processing time
#
# @author Tom Eichlersmith, University of Minnesota
###############################################################################

from LDMX.SimApplication import simcfg

###############################################################################
# @func targetBremFilter
# Returns a filter that only considers events with a hard brem
# The 'hard' cutoff is set to 1500 MeV (the max a recoil electron can have)
###############################################################################
def targetBremFilter( ) :
    target_brem_filter = simcfg.UserAction("targetHardBrem", "ldmx::TargetBremFilter")
    target_brem_filter.parameters['volume'] = 'target_PV'
    target_brem_filter.parameters['recoilEnergyThreshold'] = 1500.
    target_brem_filter.parameters['bremEnergyThreshold'] = 2500.
    return target_brem_filter

###############################################################################
# @func ecalPNFilter
# Returns a filter that only considers events a PN interaction happens inside 
#   of the ECal
###############################################################################
def ecalPNFilter( ) :
    ecal_process_filter = simcfg.UserAction("ecalPN", "ldmx::EcalProcessFilter")
    ecal_process_filter.parameters['process'] = 'photonNuclear'
    ecal_process_filter.parameters['volume'] = 'ecal'
    return ecal_process_filter

###############################################################################
# @func targetENFilter
# Returns a filter that only considers events a EN interaction happens inside 
#   of the target
###############################################################################
def targetENFilter( ) :
    target_process_filter = simcfg.UserAction("targetEN", "ldmx::TargetENProcessFilter")
    target_process_filter.parameters['process'] = 'electronNuclear'
    target_process_filter.parameters['volume'] = 'target'
    target_process_filter.parameters['recoilThreshold'] = 4000 #MeV
    return target_process_filter

###############################################################################
# @func targetPNFilter
# Returns a filter that only considers events a PN interaction happens inside 
#   of the target
###############################################################################
def targetPNFilter( ) :
    target_process_filter = simcfg.UserAction("targetPN", "ldmx::TargetProcessFilter")
    target_process_filter.parameters['process'] = 'photonNuclear'
    target_process_filter.parameters['volume'] = 'target'
    #target_process_filter.parameters['photonThreshold'] = 2500. #MeV NOT IMPLEMENTED
    return target_process_filter

###############################################################################
# @func targetDarkFilter
# Returns a filter that only considers events that have a dark brem
#   happen inside of the target
###############################################################################
def targetDarkFilter( ) :
    target_process_filter = simcfg.UserAction("targetDark", "ldmx::DarkBremFilter")
    target_process_filter.parameters['volume'] = 'target'
    return target_process_filter
