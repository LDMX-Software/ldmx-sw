"""Biasing operator templates for use throughout ldmx-sw

Mainly focused on reducing the number of places that certain parameter and class names
are hardcoded into the python configuration.
"""

from LDMX.Framework import _register, field, parameter_set


class XsecBiasingOperator:
    """required base class for biasing operators for type-checking"""

    pass


def biasing_operator(class_name, module_name="SimCore_BiasOperators"):
    return parameter_set(
        class_name=field(default=class_name, init=False),
        module_name=field(default=class_name, init=False),
        instance_name=class_name,
        post_init=lambda self: _register.library(self.module_name),
        required_base=XsecBiasingOperator,
    )


@biasing_operator("simcore::biasoperators::PhotoNuclear")
class PhotoNuclear(XsecBiasingOperator):
    """Bias photo nuclear process

    Parameters
    ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to mutliply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    down_bias_conv : bool, optional
        Should we down bias the gamma conversion process?
    only_children_of_primary : bool, optional
        Should we only bias photons that are children of the primary particle?
    """

    volume: str
    factor: float
    threshold: float = 0.0
    down_bias_conv: bool = True
    only_children_of_primary: bool = False

    def __post_init__(self):
        self.instance_name = f"{self.volume}_bias_pn"


@biasing_operator("simcore::biasoperators::ElectroNuclear")
class ElectroNuclear(XsecBiasingOperator):
    """Bias electro nuclear process

    Parameters
    ----------
    volume : str
        name of volume to bias within
    factor : float
        biasing factor to mutliply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    volume: str
    factor: float
    threshold: float = 0.0

    def __post_init__(self):
        self.instance_name = f"{self.volume}_bias_en"


@biasing_operator("simcore::biasoperators::GammaToMuPair")
class GammaToMuPair(XsecBiasingOperator):
    """Bias gamma -> mu+ mu- process

    Parameters
    ----------
    volume : str
        name of volume to bias within
    factor : float
        biasing factor to multiply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    volume: str
    factor: float
    threshold: float = 0.0

    def __post_init__(self):
        self.instance_name = f"{self.volume}_bias_mumu"


class NeutronInelastic(XsecBiasingOperator):
    """Bias neutron inelastic collisions

    Parameters
    ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to multiply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    def __init__(self, vol, factor, thresh=0.0):
        super().__init__(
            f"{vol}_bias_neutroninelastic", "simcore::biasoperators::NeutronInelastic"
        )

        self.volume = vol
        self.factor = factor
        self.threshold = thresh


class K0LongInelastic(XsecBiasingOperator):
    """Bias K0 Long inelastic collisions

    Parameters
            ----------
    vol : str
        name of volume to bias within
    factor : float
        biasing factor to multiply by
    threshold : float, optional
        minimum kinetic energy [MeV] to bias tracks
    """

    def __init__(self, vol, factor, thresh=0.0):
        super().__init__(
            f"{vol}_bias_k0longinelastic", "simcore::biasoperators::K0LongInelastic"
        )

        self.volume = vol
        self.factor = factor
        self.threshold = thresh


class DarkBrem(XsecBiasingOperator):
    """Bias dark brem process

    Parameters
    ----------
    vol : str
        name of volume to bias within
    bial_all : bool
        Should we bias all electrons or just the primary?
    factor : float
        biasing factor to mutliply by
    """

    def __init__(self, vol, bias_all, factor):
        super().__init__(f"{vol}_bias_darkbrem", "simcore::biasoperators::DarkBrem")

        self.volume = vol
        self.bias_all = bias_all
        self.factor = factor

    def ecal(self):
        """Bias dark brem inside of the ecal by the input factor"""
        return DarkBrem("ecal", True, self)

    def target(self):
        """Bias dark brem inside of the target by the input factor"""
        return DarkBrem("target_region", False, self)
