"""Python module for configuring translators"""

from LDMX.Framework import ldmxcfg

class Translator() :
    """Base class for all translator configuration classes

    Does the bare-minimum of including the library the translators
    are compiled into and defining the class name that is necessary
    to determine which C++ translator to configure.
    """

    def __init__(self, class_name, module_name = 'Packing::Translators') :
        ldmxcfg.Process.addModule(module_name)
        self.class_name = class_name

class EventHeader(Translator) :
    """Configuration for EventHeader Translator configuration"""

    def __init__(self) :
        super().__init__('packing::translators::EventHeader','Packing::Translators')
        self.i_run = 0
        self.i_event = 1
        self.i_time = 2
        self.extra_param_names = []

class Hgcroc(Translator) :
    """Configuration for Hgcroc Translator configuration

    Attributes
    ----------
    roc_version : int
        The version of the HGC ROC that is being used.
        A few small changes to the data format were implemented between ROC v2 and v3,
        so we expect this parameter to matter eventually.
    """

    def __init__(self) :
        super().__init__('packing::translators::Hgcroc','Packing::Translators')
        self.roc_version = 2
