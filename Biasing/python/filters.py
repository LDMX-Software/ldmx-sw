""" Examples of how each filter is configured for biased MC generation. 

    The configurations below reflect the nominal values currently being used 
    for large scale production.  When debugging, it's best to create these
    generators directly. 

"""

from LDMX.SimApplication import simcfg

def target_brem_filter():
    """ Configuration for filtering events that don't see a hard brem in the target. 

    An event is vetoed if one of two conditions is satisfied:
    1) The recoil electron exits the target area with an energy above 1500 MeV
    2) The recoil electorn brems, but the energy of at least one of the brems
    isn't above 2500 MeV. 

    Returns
    -------
    Instance of configured TargetBremFilter.

    """
    target_brem_filter = simcfg.UserAction("target_brem_filter", "ldmx::TargetBremFilter")
    target_brem_filter.parameters['recoil_max_p_threshold'] = 1500.
    target_brem_filter.parameters['recoil_min_energy_threshold'] = 2500.
    return target_brem_filter

def ecal_pn_filter():
    """ Configuration for filtering events that don't see a hard brem undergo a photo-nuclear reaction in the ECal. 

    Returns
    -------
    Instance of configured EcalProcessFilter.

    """
    ecal_process_filter = simcfg.UserAction("ecal_process_filter", "ldmx::EcalProcessFilter")
    ecal_process_filter.parameters['process'] = 'photonNuclear'
    return ecal_process_filter

def target_en_filter():
    """ Configuration for filtering electro-nuclear events in the target. 

    Returns
    -------
    Instance of configured TargetENProcessFilter.

    """
    target_process_filter = simcfg.UserAction("target_en_process_filter", "ldmx::TargetENProcessFilter")
    target_process_filter.parameters['process'] = 'electronNuclear'
    target_process_filter.parameters['volume'] = 'target'
    target_process_filter.parameters['recoilThreshold'] = 4000 #MeV
    return target_process_filter

def target_pn_filter():
    """ Configuration for filtering photo-nuclear events in the target. 

    Returns
    -------
    Instance of configured TargetProcessFilter.

    """
    target_process_filter = simcfg.UserAction("target_process_filter", "ldmx::TargetProcessFilter")
    target_process_filter.parameters['process'] = 'photonNuclear'
    return target_process_filter

def target_ap_filter():
    """ Configuration for filtering A' events in the target. 

    Returns
    -------
    Instance of configured DarkBremFilter.

    """

    target_ap_filter = simcfg.UserAction("target_ap_filter", "ldmx::DarkBremFilter")
    target_ap_filter.parameters['volume'] = 'target'
    return target_ap_filter

def tagger_veto_filter(): 
    """ Configuration used to reject off-energy electrons in the tagger tracker.

    Returns
    -------
    Instance of configured TaggerVetoFilter

    """
    
    tagger_veto_filter = simcfg.UserAction("tagger_veto_filter", "ldmx::TaggerVetoFilter")
    tagger_veto_filter.parameters['threshold'] = 3800.
    return tagger_veto_filter

def track_filter(process_name):
    """ Configuration used to tag all tracks produced via the given process to persist them to the event.

    Parameters
    ----------
    process_name : str
        The Geant4 process name (e.g. photonNuclear) via which the tracks were 
        produced. 

    Return
    ------
    Instance of TrackProcessFilter configured to tag tracks produced via the 
    given process. 


    """
    track_filter = simcfg.UserAction('%s_track_filter' % process_name, 'ldmx::TrackProcessFilter')
    track_filter.parameters['process'] = process_name
    return track_filter


def pn_track_filter():
    """ Configuration used to tag all photo-nuclear tracks to persist them to the event. 

    Return
    ------
    Instance of TrackProcessFilter configured to tag photo-nuclear tracks.

    """
    return track_filter('photonNuclear')


def en_track_filter():
    """ Configuration used to tag all electro-nuclear tracks to persist them to the event. 

    Return
    ------
    Instance of TrackProcessFilter configured to tag electro-nuclear tracks.

    """
    return track_filter('electronNuclear')

def ap_track_filter():
    """ Configuration used to tag all dark brem tracks to persist them to the event. 

    Return
    ------
    Instance of TrackProcessFilter configured to tag dark brem tracks.

    """
    return track_filter('eDBrem')
