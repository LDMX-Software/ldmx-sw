"""Configuration module for dark brem simulation"""

class DarkBremModel() :
    """Storage for parameters of a dark brem model

    All other models should inherit from this class
    in order to keep the correct internal parameters.

    Parameters
    ----------
    name : str
        Name of this dark brem model
    """

    def __init__(self,name) :
        self.name = name

    def __str__(self) :
        string = '%s {'%self.name
        for key in self.__dict__ :
            if key is not self.name :
                string += ' %s=%s'%( key , self.__dict__[key] )
        string += ' }'
        return string

class G4DarkBreMModel(DarkBremModel) :
    """Configuration for the event library dark brem model

    This model uses G4DarkBreM's library model. The library
    can be a directory of LHE files, gzip-compressed LHE files,
    a CSV processed by G4DarkBreM, or a gzip-compressed CSV
    processed by G4DarkBreM.

    Parameters
    ----------
    library_path : str
        Path to library holding the dark brem kinematics

    Attributes
    ----------
    method : str
        Interpretation method for LHE files
    threshold : float
        Minimum energy [GeV] that electron should have for dark brem to have nonzero xsec
    epsilon : float
        Epsilon for dark brem xsec calculation
    """

    def __init__(self, library_path) :
        super().__init__('g4db')
        self.library_path = library_path
        self.method       = 'forward_only'
        self.threshold    = 2.0 #GeV
        self.epsilon      = 0.01

# for legacy reasons, we define another name for the G4DB model
VertexLibraryModel = G4DarkBreMModel

class DarkBrem:
    """Storage for parameters of dark brem process

    Attributes
    ----------
    ap_mass : float
        Mass of A' in MeV
    enable : bool
        Should we use the custom Geant4 dark brem process? (Default: No)
    only_one_per_event : bool
        Should we deactivate the process after one dark brem or allow for more than one? (Default: No)
    cache_xsec : bool
        Should we cache the xsec's computed from the model? (Default: yes)
    model : DarkBremModel
        The model that should be use for dark bremming
    """

    def __init__(self) : 
        self.ap_mass            = 0.
        self.only_one_per_event = False
        self.enable             = False #off by default
        self.cache_xsec         = True
        self.model              = DarkBremModel('UNDEFINED')

    def activate(self, ap_mass, model = None) :
        """Activate the dark brem process with the input A' mass [MeV] and dark brem model

        If no dark brem model is given, we do not activate the process
        and only define the A' mass. This allows for some backwards
        compatibility by allowing users to use the LHEPrimaryGenerator
        with A' particles.
        """

        self.ap_mass = ap_mass

        if model is not None :
            if not isinstance(model,DarkBremModel) :
                raise Exception('Dark brem process needs to be configured with an associated DarkBremModel.')
    
            self.enable = True
            self.model  = model

    def __str__(self): 
        """Stringify the DarkBrem configuration

        Returns
        -------
        str
            A human-readable version of all its attributes
        """

        string  = "{ Enabled: %r"%self.enable
        if self.enable :
            string += ", Mass: %.1f MeV"%self.ap_mass
            string += ", Only One Per Event: %r"%self.only_one_per_event
            string += ", Model: %s"%self.model

        string += " }"

        return string
