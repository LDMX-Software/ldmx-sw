"""Utility classes for Simulation"""

from LDMX.SimCore import simcfg

class BiasingUtilityAction(simcfg.UserAction) :
    """Helpful derived class for this submodule, makes
    sure the library and namespace are set correctly.

    Parameters
    ----------
    instance_name : str
        name of this instance
    class_name : str
        name of the class within this submodule
    """

    def __init__(self,instance_name,class_name) :
        super().__init__(instance_name,'biasing::utility::%s'%class_name)
        from LDMX.Framework.ldmxcfg import Process
        Process.addLibrary('@CMAKE_INSTALL_PREFIX@/lib/libBiasing_Utility.so')

class StepPrinter(BiasingUtilityAction) :
    """Print each step of the input track ID

    The default track ID is 1 (the primary particle).

    Parameters
    ----------
    track_id : int, optional
        Geant4 track ID to print each step of
    """

    def __init__(self,track_id=1) :
        super().__init__('print_steps_%s'%track_id,'StepPrinter')

        self.track_id = track_id

class PartialEnergySorter(BiasingUtilityAction) :
    """Process particles such that all particles above
    the input threshold are processed first.

    Parameters
    ----------
    thresh : float
        Minimum energy [MeV] to process a track first
    """

    def __init__(self,thresh) :
        super().__init__('sort_above_%dMeV'%thresh,'PartialEnergySorter')

        self.threshold = thresh

class TrackProcessFilter(BiasingUtilityAction):
    """ Configuration used to tag all tracks produced via the given process to persist them to the event.

    Parameters
    ----------
    process_name : str
        The Geant4 process name (e.g. photonNuclear) via which the tracks were produced. 
    """

    def __init__(self,process_name) :
        super().__init__('%s_track_filter'%process_name, 'TrackProcessFilter' )

        self.process = process_name

    def photo_nuclear() :
        """ Configuration used to tag all photo-nuclear tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag photo-nuclear tracks.
        """
        return TrackProcessFilter('photonNuclear')

    def electro_nuclear() :
        """ Configuration used to tag all electro-nuclear tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag electro-nuclear tracks.
    
        """
        return TrackProcessFilter('electronNuclear')

    def dark_brem() :
        """ Configuration used to tag all dark brem tracks to persist them to the event. 
    
        Return
        ------
        Instance of TrackProcessFilter configured to tag dark brem tracks.
    
        """
        return TrackProcessFilter('eDarkBrem')

class DecayChildrenKeeper(BiasingUtilityAction):
    """ Configuration used to store children of specific particle decays

    Parameters
    ----------
    parents : list[int]
        list of PDG ID of particles whose decay products we want to keep
    """

    def __init__(self,parents) :
        super().__init__('keep_decay_children', 'DecayChildrenKeeper' )

        self.parents = parents
