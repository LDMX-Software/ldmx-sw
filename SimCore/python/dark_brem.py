"""Configuration module for dark brem simulation"""

from LDMX.Framework import field, parameter_set


@parameter_set
class DarkBremModel:
    """base class for dark brem models for type checking"""

    pass


def dark_brem_model(name):
    return parameter_set(name=name, required_base=DarkBremModel)


@dark_brem_model("UNDEFINED")
class UndefinedModel(DarkBremModel):
    pass


@dark_brem_model("g4db")
class G4DarkBreMModel(DarkBremModel):
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
        Minimum energy [GeV] that electron should have for dark brem
        to have nonzero xsec
    epsilon : float
        Epsilon for dark brem xsec calculation
    """

    library_path: str
    method: str = "forward_only"
    threshold: float = 2.0
    epsilon: float = 0.01


@parameter_set
class DarkBrem:
    """Storage for parameters of dark brem process

    Attributes
    ----------
    ap_mass : float
        Mass of A' in MeV
    enable : bool
        Should we use the custom Geant4 dark brem process?
        (Default: No)
    only_one_per_event : bool
        Should we deactivate the process after one dark brem or allow for more than one? (Default: No)
    cache_xsec : bool
        Should we cache the xsec's computed from the model? (Default: yes)
    model : DarkBremModel
        The model that should be use for dark bremming
    """

    ap_mass: float = 0.0
    only_one_per_event: bool = False
    enable: bool = False
    cache_xsec: bool = True
    model: DarkBremModel = UndefinedModel()

    def activate(self, ap_mass, model=None):
        """Activate the dark brem process with the input A' mass [MeV]
        and dark brem model

        If no dark brem model is given, we do not activate the process
        and only define the A' mass. This allows for some backwards
        compatibility by allowing users to use the LHEPrimaryGenerator
        with A' particles.
        """

        self.ap_mass = ap_mass

        if model is not None:
            self.enable = True
            self.model = model
