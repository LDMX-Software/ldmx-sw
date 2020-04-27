
from LDMX.SimApplication import simcfg

def targetBremFilter( ) :
    target_brem_filter = simcfg.UserAction("targetHardBrem", "ldmx::TargetBremFilter")
    target_brem_filter.parameters['volume'] = 'target_PV'
    target_brem_filter.parameters['recoilEnergyThreshold'] = 1500.
    target_brem_filter.parameters['bremEnergyThreshold'] = 2500.
    return target_brem_filter

def ecal_pn_filter( ) :
    ecal_process_filter = simcfg.UserAction("ecalPN", "ldmx::EcalProcessFilter")
    ecal_process_filter.parameters['process'] = 'photonNuclear'
    return ecal_process_filter

def targetENFilter( ) :
    target_process_filter = simcfg.UserAction("targetEN", "ldmx::TargetENProcessFilter")
    target_process_filter.parameters['process'] = 'electronNuclear'
    target_process_filter.parameters['volume'] = 'target'
    target_process_filter.parameters['recoilThreshold'] = 4000 #MeV
    return target_process_filter

def targetPNFilter( ) :
    target_process_filter = simcfg.UserAction("targetPN", "ldmx::TargetProcessFilter")
    target_process_filter.parameters['process'] = 'photonNuclear'
    target_process_filter.parameters['volume'] = 'target'
    #target_process_filter.parameters['photonThreshold'] = 2500. #MeV NOT IMPLEMENTED
    return target_process_filter

def targetDarkFilter( ) :
    target_process_filter = simcfg.UserAction("targetDark", "ldmx::DarkBremFilter")
    target_process_filter.parameters['volume'] = 'target'
    return target_process_filter
