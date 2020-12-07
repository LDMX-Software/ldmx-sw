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

class WeightByStep(BiasingUtilityAction) :
    """Calculate the event weight by mutliplying all step weights together.

    There are no parameters for this action.
    Either you use it or you don't.
    """

    def __init__(self) :
        super().__init__('weight_by_step','WeightByStep')

