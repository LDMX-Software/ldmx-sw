""" filters
Examples of how each filter is configured for biased MC generation. 

    The configurations below reflect the nominal values currently being used 
    for large scale production.  When debugging, it's best to create these
    generators directly. 
"""

from LDMX.SimCore import simcfg

class TargetBremFilter(simcfg.UserAction):
    """ Configuration for filtering events that don't see a hard brem in the target. 

    An event is vetoed if one of two conditions is satisfied:
    1) The recoil electron exits the target area with an energy above 1500 MeV
    2) The recoil electorn brems, but the energy of at least one of the brems
    isn't above 2500 MeV. 

    Parameters
    ----------
    recoil_max_p : float
        Maximum momentum the recoil electron can have [MeV]
    brem_min_e : float
        Minimum energy the brem photon can have [MeV]

    Attributes
    ----------
    kill_recoil_track : bool
        Should we kill the recoil electron track for a worst case scenario?
    """

    def __init__(self,recoil_max_p = 1500.,brem_min_e = 2500.) :
        super().__init__("target_brem_filter", "biasing::TargetBremFilter")

        from LDMX.Biasing import include
        include.library()

        self.recoil_max_p_threshold = recoil_max_p
        self.brem_min_energy_threshold = brem_min_e
        self.kill_recoil_track = False

class EcalProcessFilter(simcfg.UserAction):
    """ Configuration for filtering events that don't see a hard brem undergo a photo-nuclear reaction in the ECal. 

    Parameters
    ----------
    process : str
        Geant4 process to look for in the ecal
    """

    def __init__(self,process = 'photonNuclear') :
        super().__init__('ecal_%s_filter'%process,'biasing::EcalProcessFilter')

        from LDMX.Biasing import include
        include.library()

        self.process = process

class TargetENFilter(simcfg.UserAction) :
    """ Configuration for filtering electro-nuclear events in the target. 

    Parameters
    ----------
    recoil_thresh : float
        Maximum energy recoil electron is allowed to have [MeV]
    """

    def __init__(self,recoil_thresh = 2500.) :
        super().__init__("target_en_process_filter","biasing::TargetENProcessFilter")

        from LDMX.Biasing import include
        include.library()

        self.recoilThreshold = recoil_thresh #MeV

class TargetPNFilter(simcfg.UserAction) :
    """ Configuration for filtering photo-nuclear events in the target."""

    def __init__(self) :
        super().__init__("target_process_filter", "biasing::TargetProcessFilter")

        from LDMX.Biasing import include
        include.library()

        self.process = 'photonNuclear'

class EcalDarkBremFilter(simcfg.UserAction):
    """ Configuration for filtering A' events

    Parameters
    ----------
    minApEnergy : float
        Minimum A' energy to keep the event [MeV]
    """

    def __init__(self,minApEnergy):
        super().__init__('ecal_db_filter','biasing::EcalDarkBremFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = minApEnergy

class TargetDarkBremFilter(simcfg.UserAction):
    """ Configuration for filtering A' events

    Parameters
    ----------
    minApEnergy : float
        Minimum A' energy to keep the event [MeV]
    """

    def __init__(self,minApEnergy):
        super().__init__('target_db_filter','biasing::TargetDarkBremFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = minApEnergy

class TaggerVetoFilter(simcfg.UserAction): 
    """ Configuration used to reject off-energy electrons in the tagger tracker.

    Parameters
    ----------
    thresh : float
        Minimum energy [MeV] that electron should have
    """
    
    def __init__(self,thresh=3800.) :
        super().__init__('tagger_veto_filter','biasing::TaggerVetoFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = thresh

class PrimaryToEcalFilter(simcfg.UserAction) :
    """ Configuration used to reject events where the primary doesn't reach the ecal with a mimimum energy

    Parameters
    ----------
    thresh : float
        Minimum energy [MeV] that primary electron should have when hitting ecal
    """

    def __init__(self,thresh) :
        super().__init__('primary_to_ecal_with_%d'%thresh,'biasing::PrimaryToEcalFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = thresh

class MidShowerNuclearBkgdFilter(simcfg.UserAction) :
    """ Configuration used to reject events that don't have enough energy given to the input process.

    Parameters
    ----------
    thresh : float
        Minimum energy [MeV] that the kinetic energy of the products needs to sum to
    """

    def __init__(self,thresh) :
        super().__init__('midshower_nuclear_min_%d_MeV'%(thresh),'biasing::MidShowerNuclearBkgdFilter')

        from LDMX.Biasing import include
        include.library()

        self.threshold = thresh
